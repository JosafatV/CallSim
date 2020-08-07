#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <mpi.h>
   
#define max_rows 1000000
#define send_data_tag 2001
#define return_data_tag 2002

typedef struct {
	char node;
	char status;
	unsigned short wait;
} call_t;

void* say_hello (void* msg) {
	char* pmsg = (char*) msg;
	printf("Hello world from %s\n", pmsg);
}

int call_counter = 0;
int next_call = 0;
int stop_threads = 0;
call_t allCalls[max_rows];
call_t rcvCall;
int allWorkers[max_rows];

int main(int argc, char **argv) {
	MPI_Status status;
	int my_id, root_process, ierr, i, j, num_rows, num_procs, alpha, 
		an_id, num_rows_to_receive, avg_rows_per_process, 
		sender, num_rows_received, start_row, end_row, num_rows_to_send;

  /* Now replicte this process to create parallel processes.
   * From this point on, every process executes a seperate copy
   * of this program */
	ierr = MPI_Init(&argc, &argv);
	root_process = 0;
  
	/* find out MY process ID, and how many processes were started. */
	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

// ===================== HOST PROCESS ===================== //
	if (my_id == root_process) {
		int num_rows = 10;

		// initialize the arrays (move to parallel function)
		for(i = 0; i < num_rows; i++) {
			allCalls[i].node = '0';
			allCalls[i].status = 'p';
			allCalls[i].wait = 0;
		}

		for (an_id = 1; an_id < num_procs; an_id++) {
			ierr = MPI_Send(&allCalls[an_id], 1, MPI_INT,
				an_id, send_data_tag, MPI_COMM_WORLD);
		}

		// Calculate the values in the segment assigned to the root process
			allCalls[0].wait = 10;
			allCalls[0].status = 'c';
			char* msg = "host";
			say_hello((void*) msg);

		// Receive the partial results from every process
		for(an_id = 1; an_id < num_procs; an_id++) {
			
			ierr = MPI_Recv(&rcvCall, 1, MPI_INT, MPI_ANY_SOURCE,
				return_data_tag, MPI_COMM_WORLD, &status);

			sender = status.MPI_SOURCE;
			
			// Merge results
			allCalls[sender] = rcvCall;
			
		}
		
		printf("+++ TEST COMPLETED +++ \n");//, num_rows, omp_get_wtime()-start);

		// Display the final result to the screen
		for(i = 0; i < num_rows; i++) {
			printf("node = %c | status = %c | wait_time = %d\n", allCalls[i].node, allCalls[i].status, allCalls[i].wait);
		}

	} else {

	 // ===================== CLIENT PROCESS ===================== //

	 ierr = MPI_Recv( &rcvCall, 1, MPI_INT, 
		   root_process, send_data_tag, MPI_COMM_WORLD, &status);

	// num_rows_received = num_rows_to_receive;

	// Calculate the node's portion of the array
	rcvCall.status = 'c';
	rcvCall.wait = 9;
	char* msg = "node";
	say_hello((void*) msg);

	 // Send partial result back to master
	 ierr = MPI_Send( &rcvCall, 1, MPI_INT, root_process, 
		   return_data_tag, MPI_COMM_WORLD);
  }

  ierr = MPI_Finalize();
}
