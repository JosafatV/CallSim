CC = mpicc
EX = mpiexec
CP = cp
DT = /mirror

FLAGS = -pthread

SRC_P = callt_mpi.c
EXE_P = callt


test:
		$(CC) $(SRC_P) -o $(EXE_P) $(FLAGS)
		mv $(EXE_P) $(DT)