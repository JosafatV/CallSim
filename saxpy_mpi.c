#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>
   
#define max_rows 10000000
#define send_data_tag 2001
#define return_data_tag 2002

int arrayZ[max_rows];
int arrayA[max_rows];
int arrayX[max_rows];
int arrayY[max_rows];

int array2Z[max_rows];
int array2A[max_rows];
int array2X[max_rows];
int array2Y[max_rows];

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
	if(my_id == root_process) {

		printf("please enter the number of numbers to sum: ");
		scanf("%i", &num_rows);

		if(num_rows > max_rows) {
			printf("Too many numbers, using max_rows.\n");
			num_rows = max_rows;
			//exit(1);
		}

		printf("please enter the value of alpha: ");
		scanf("%i", &alpha);

		if(alpha > 65535) {
			printf("alpha may cause overflow.\n");
		}

		avg_rows_per_process = num_rows / num_procs;

		// initialize the arrays

		for(i = 0; i < num_rows; i++) {
			arrayA[i] = alpha;
			arrayX[i] = rand()%256;
			arrayY[i] = rand()%1024;
		}
		
		double start = omp_get_wtime();

		// distribute a portion of the vector to each child process

		for(an_id = 1; an_id < num_procs; an_id++) {
			start_row = an_id*avg_rows_per_process;// + 1;
			end_row   = (an_id + 1)*avg_rows_per_process;

			if((num_rows - end_row) < avg_rows_per_process)
				end_row = num_rows - 1;

			num_rows_to_send = end_row - start_row + 1;

			ierr = MPI_Send( &num_rows_to_send, 1 , MPI_INT,
				an_id, send_data_tag, MPI_COMM_WORLD);

			ierr = MPI_Send( &arrayA[start_row], num_rows_to_send, MPI_INT,
				an_id, send_data_tag, MPI_COMM_WORLD);

			ierr = MPI_Send( &arrayX[start_row], num_rows_to_send, MPI_INT,
				an_id, send_data_tag, MPI_COMM_WORLD);

			ierr = MPI_Send( &arrayY[start_row], num_rows_to_send, MPI_INT,
				an_id, send_data_tag, MPI_COMM_WORLD);
		}

		// Calculate the values in the segment assigned to the root process
		for(i = 0; i < avg_rows_per_process; i++) {
			arrayZ[i] = arrayA[i] * arrayX[i] + arrayY[i];   
		}

		// Receive the partial results from every process
		for(an_id = 1; an_id < num_procs; an_id++) {
			
			ierr = MPI_Recv(&array2Z, num_rows_to_send+1, MPI_INT, MPI_ANY_SOURCE,
				return_data_tag, MPI_COMM_WORLD, &status);

			sender = status.MPI_SOURCE;
			
			// Merge results
			j = 0;
			for(i = sender*avg_rows_per_process; i < sender*avg_rows_per_process+avg_rows_per_process; i++) {
				//data comes at the beginning of the array, j points to this data, i to where it should go
				arrayZ[i] = array2Z[j];
				j++;
			}
		}
		
		printf("+++ SAXPY with LIST_SIZE %d took: %.7f +++ \n", num_rows, omp_get_wtime()-start);

		// Display the final result to the screen
		/*
		for(i = 0; i < num_rows; i++) {
			printf("%d * %d + %d = %d\n", arrayA[i], arrayX[i], arrayY[i], arrayZ[i]);
		}*/

	} else {

	 // ===================== CLIENT PROCESS ===================== //

	 ierr = MPI_Recv( &num_rows_to_receive, 1, MPI_INT, 
		   root_process, send_data_tag, MPI_COMM_WORLD, &status);
	  
	 ierr = MPI_Recv( &array2A, num_rows_to_receive, MPI_INT, 
		   root_process, send_data_tag, MPI_COMM_WORLD, &status);

	 ierr = MPI_Recv( &array2X, num_rows_to_receive, MPI_INT, 
		   root_process, send_data_tag, MPI_COMM_WORLD, &status);

	 ierr = MPI_Recv( &array2Y, num_rows_to_receive, MPI_INT, 
		   root_process, send_data_tag, MPI_COMM_WORLD, &status);

	 num_rows_received = num_rows_to_receive;

	 // Calculate the node's portion of the array
	 for(i = 0; i < num_rows_received; i++) {
		array2Z[i] = array2A[i] * array2X[i] + array2Y[i];
	 }

	 printf("Print from client\n");

	 // Send partial result back to master
	 ierr = MPI_Send( &array2Z[0], num_rows_received, MPI_INT, root_process, 
		   return_data_tag, MPI_COMM_WORLD);
  }

  ierr = MPI_Finalize();
}