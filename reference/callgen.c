#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

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

int main() {
    
    int day = 7;
    int tod = 48;
    int lun, mar, mie, jue, vie, sab, dom;
	
    srand(time(0));
	printf("ToD\tLun\tMar\tMie\tJue\tVie\tSab\tDom\n");
	//for (int i=0; i<day; i++)
	for (int j=0; j<tod; j++){
		lun = generate_calls(0, j);
		mar = generate_calls(1, j);
		mie = generate_calls(2, j);
		jue = generate_calls(3, j);
		vie = generate_calls(4, j);
		sab = generate_calls(5, j);
		dom = generate_calls(6, j);
		printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n", dom, lun, mar, mie, jue, vie, sab);
	}
	
	return 0;
}
