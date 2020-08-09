Instituto Tecnológico de Costa Rica
Ingeniería en Computadores
Arquitectura de Computadores II
# Proyecto3: CallSim

Call Center Simulation with OpenMPI and Pthreads that aims to measure the improvement in performance of a cluster versus a single computer

### Compiling and running

Compile the file with 
```
mpicc callsim.c -o callSim -pthread -fopenmp
```
Copy the executable to /mirror
```
sudo cp callSim /mirror/callSim
```
	
Execute the file with 
```
mpiexec -n 8 -f machinefile ./callSim
```

- -n 8 defines the amount of processes
- -f machinefiles defines the addresses for the nodes and the split of the processes
	
You can also run make to compile and copy the executable file
```
make
```
