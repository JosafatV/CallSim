#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <omp.h>
#include <mpi.h>
   
#define max_rows 100000000
#define send_data_tag 2001
#define return_data_tag 2002
#define second 1000000
#define s_minute 10 // simulated minute
#define waiting 'w'
#define processing 'p'
#define complete 'c'

typedef struct {
	char node;
	char status;
	unsigned short wait;
} call_t;

call_t allCalls[max_rows];
pthread_t allWorkers[100];
pthread_mutex_t access_lock;
int next_call = 0;		// Next call
int call_counter = 0;	// Append and length
int stop_threads = 0;
int max_workers = 50;
int max_period = 49*100; // 49*1000
int rcvMiss = 0;
int rcvAll = 0;

/** Generate a random number of calls based on the erlang
 * \param day which day of the week is it? 0 == monday
 * \param period which period of the days is it? 0 == 0:00-0:30
 * \return a pseudo-random number greater than 0
 */
int generate_calls(int day, int period) {
	int avg, stddev;
	
	if (day > 4) {
		// Weekend
		if (period < 6) {
			avg = 11;
			stddev = 2;
		} else if (period < 12) {
			avg = 25;
			stddev = 8;
		} else if (period < 18) {
			avg = 51;
			stddev = 13;
		} else if (period < 24) {
			avg = 78;
			stddev = 15;
		} else if (period < 30) {
			avg = 89;
			stddev = 9;
		} else if (period < 36) {
			avg = 80;
			stddev = 12;
		} else if (period < 42) {
			avg = 49;
			stddev = 12;
		} else {
			avg = 23;
			stddev = 6;
		}
	} else {
		// Workday
		if (period < 6) {
			avg = 12;
			stddev = 3;
		} else if (period < 12) {
			avg = 33;
			stddev = 15;
		} else if (period < 18) {
			avg = 139;
			stddev = 39;
		} else if (period < 24) {
			avg = 226;
			stddev = 22;
		} else if (period < 30) {
			avg = 247;
			stddev = 19;
		} else if (period < 36) {
			avg = 183;
			stddev = 32;
		} else if (period < 42) {
			avg = 73;
			stddev = 25;
		} else {
			avg = 24;
			stddev = 7;
		}
	}
	int calls = rand()%stddev-(stddev/2);
	return calls + avg;

}

/** Create a number of calls based on statistics while there is room
 * \param avgCalls the average amount of calls in the time frame
 * \param stdDev the standard deviation of calls to add randomness
 */
void* createCall( int day, int period){
	int newCalls = generate_calls(day, period);
	//printf("Generating %d calls\n", newCalls);
	for (int i=0; i<newCalls; i++){
		call_t nc = {'a', waiting, 0};
		if (call_counter<max_rows){
			allCalls[call_counter] = nc;
			call_counter++;
		}
	}
}

/** Main thread cycle that updates calls. It simulates the 
 * work made by the employees.
 */
void* processCall() {
	int current_call = 0;
	while (stop_threads == 0) {
		pthread_mutex_lock(&access_lock);
		current_call = next_call;
		next_call++;
		pthread_mutex_unlock(&access_lock);
		// If the call doesn't exist: wait
		while (current_call>=call_counter) {
			//printf("Waiting for a call\n");
			usleep(s_minute);
			if (stop_threads){ break; }
		}
		if (stop_threads == 0) {
			//printf("Working on call %d\n", current_call);
			allCalls[current_call].status = processing;
			usleep((rand()%5+2)*s_minute); // TODO change to efficiency or statistics
			allCalls[current_call].status = complete;
		} else {
			break;
		}
	}
}


int main(int argc, char** argv) {
	MPI_Status status;
	int my_id, root_process, ierr, i, j, num_rows, num_procs, alpha, 
		an_id, num_rows_to_receive, avg_rows_per_process, 
		sender, num_rows_received, start_row, end_row, num_rows_to_send;
    int myrank, nprocs;

    ierr = MPI_Init(&argc, &argv);
	root_process = 0;
	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
	int ret = 0;
	srand(time(0));

    MPI_Get_processor_name(processor_name, &name_len);
    //printf("+++ Starting %s process %d of %d +++\n", processor_name, my_id+1, num_procs);

// ===================== HOST PROCESS ===================== //
	if (my_id == root_process) {

		double start = omp_get_wtime();

		// Create a thread for each worker
		for (int w = 0; w<max_workers; w++) {
			ret = pthread_create (&allWorkers[w], NULL, processCall, NULL);
			if (ret!=0) {printf("Error creating worker thread\n");}
		}

		// create calls depending on the period for 1 day
		for (int period=0; period<max_period; period++){
			// Create or remove workers based on the period

			// Create calls (TODO: stagger)
			createCall(0, period); // Always monday
			usleep(30*s_minute); // Each period is 30m
		}

		// work day is over
		stop_threads = 1;
		long totalCalls = call_counter-1;

		// join threads
		for (int w = 0; w<max_workers; w++) {
			pthread_join (allWorkers[w], NULL);
		}

		//Tally up the data
		long missed_calls = 0;
		for (int check=call_counter-1; check>0; check--) {
			if(allCalls[check].status != complete){
				missed_calls++;
			} else {
				break;
			}
		}
		//printf("Missed %d calls from 0\n", rcvMiss);

		// Receive data from other nodes
		for(an_id = 1; an_id < num_procs; an_id++) {
			ierr = MPI_Recv(&rcvAll, 1, MPI_INT, MPI_ANY_SOURCE,
				return_data_tag, MPI_COMM_WORLD, &status);


			ierr = MPI_Recv(&rcvMiss, 1, MPI_INT, MPI_ANY_SOURCE,
				return_data_tag, MPI_COMM_WORLD, &status);

			sender = status.MPI_SOURCE;

			// Merge results
			totalCalls += rcvAll;
			missed_calls += rcvMiss;
			//printf("Missed %i from process %i\n", rcvMiss, sender);
		}

		printf("+++ Total calls  %ld +++\n", totalCalls);
		printf("+++ Missed calls %ld +++\n", missed_calls);
		printf("+++ callSim with %d periods and %d workers took : %.7f +++ \n", max_period, max_workers, omp_get_wtime()-start);

	} else {
// ===================== CLIENT PROCESS ===================== //
		for (int w = 0; w<max_workers; w++) {
			ret = pthread_create (&allWorkers[w], NULL, processCall, NULL);
			if (ret!=0) {printf("Error creating worker thread\n");}
		}

		// create calls depending on the period for 1 day
		for (int period=0; period<max_period; period++){
			// Create or remove workers based on the period

			// Create calls (TODO: stagger)
			createCall(0, period); // Always monday
			usleep(30*s_minute); // Each period is 30m
		}

		// work day is over
		stop_threads = 1;
		//printf("+++ Total calls: %d +++\n", call_counter-1);

		// join threads
		for (int w = 0; w<max_workers; w++) {
			pthread_join (allWorkers[w], NULL);
		}

		//Tally up the data
		int missed_calls = 0;
		for (int check=call_counter-1; check>0; check--) {
			if(allCalls[check].status != complete){
				missed_calls++;
			} else {
				break;
			}
		}

		// Send data to root node
		ierr = MPI_Send( &call_counter-1, 1, MPI_INT, root_process, 
		   return_data_tag, MPI_COMM_WORLD);
		
		ierr = MPI_Send( &missed_calls, 1, MPI_INT, root_process, 
		   return_data_tag, MPI_COMM_WORLD);
  }

	ierr = MPI_Finalize();
	return 0;
}
