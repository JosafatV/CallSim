CC = mpicc
EX = mpiexec
CP = cp
DT = /mirror

FLAGS = -pthread -fopenmp

SRC_P = callsim.c
EXE_P = callSim


test:
		$(CC) $(SRC_P) -o $(EXE_P) $(FLAGS)
		sudo $(CP) $(EXE_P) $(DT)