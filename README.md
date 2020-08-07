Instituto Tecnológico de Costa Rica
Ingeniería en Computadores
Arquitectura de Computadores II
# Proyecto3: CallSim

Call Center Simulation with OpenMPI and Pthreads that aims to measure the improvement in performance of a cluster versus a single computer

### Compiling and running

Compile the file with 
```
mpicc saxpi_mpi.c -o saxpi_mpi -fopenmp
```
Copy the executable to /mirror
```
sudo cp saxpy_mpi.c /mirror/saxpy_mpi.c
```
	
Execute the file with 
```
mpiexec -n 8 -f machinefile ./saxpi_mpi
```	
- -n 8 defines the amount of processes
- -f machinefiles defines the addresses for the nodes and the split of the processes
	
