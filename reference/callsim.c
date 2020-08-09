#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
//#include <mpi.h>
   
#define max_rows 1000000
#define send_data_tag 2001
#define return_data_tag 2002
#define waiting 'w'
#define processing 'p'
#define complete 'c'

typedef struct {
	char node;
	char status;
	unsigned short wait;
} call_t;

typedef struct {
	unsigned short calls_taken;
	short efficiency;
} worker_t;

pthread_mutex_t access_lock;
int call_counter = 0;
int next_call = 0;
int call_pointer = 0;
int stop_threads = 0;

call_t allCalls[max_rows];
//call_t allCalls_node0[max_rows];

/** Create a number of calls based on statistics while there is room
 * \param avgCalls the average amount of calls in the time frame
 * \param stdDev the standard deviation of calls to add randomness
 */
void* createCall(){//int avgCalls, int stdDev) {
	int newCalls = 18;//avgCalls + rand()%stdDev-stdDev/2;
	for (int i=0; i<newCalls; i++){
		call_t nc = {'a', waiting, 0};
		if (call_pointer<max_rows){
			allCalls[call_pointer] = nc;
			call_pointer++;
		}
	}
}

/** Main thread cycle that updates calls. The work made by the
 * 	employees 
 */
void* processCall() {
	int current_call = 0;
	while (stop_threads == 0) {
		pthread_mutex_lock(&access_lock);
		current_call = next_call;
		next_call++;
		pthread_mutex_unlock(&access_lock);
		// If the call doesn't exist: wait
		while (current_call>=call_pointer) {
			printf("Waiting for a call\n");
			usleep(1000);
		}
		printf("Working on call %d\n", current_call);
		allCalls[current_call].status = processing;
		usleep(1000); // efficiency or statistics
		allCalls[current_call].status = complete;
	}
}

int main () {
	pthread_t allWorkers[1000];
	int max_workers = 20;
	int ret = 0;
	srand(time(0));

	createCall(); //so that the workers can immediately work ***(may not be realistic)
	
	// create threads
	for (int w = 0; w<max_workers; w++) {
		// append thread number to msg
		ret = pthread_create (&allWorkers[w], NULL, processCall, NULL);
		if (ret!=0) {printf("Error creating worker thread\n");}
	}

	// create calls in a cycle
	for (int i=0; i<49; i++){
		createCall();
		usleep(1000);
	}

	// work day is over
	stop_threads = 1;
	printf("+++ Total calls: %d +++\n", call_pointer-1);

	// join threads
	for (int w = 0; w<max_workers; w++) {
		pthread_join (allWorkers[w], NULL);
	}
}