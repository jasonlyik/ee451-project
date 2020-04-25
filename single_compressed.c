#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "single_compressed.h"

// See output file of rmat.py for assumed input format



void csr(cs_matrix_t *m, const char *file, int n) {
	m->column = 0;

	FILE *fp = fopen(file, "r");
	int bufc, bufr;

	int size = 1024;
	m->JC = (int *) malloc(sizeof(int) * (n+1));
	m->IR = (int *) malloc(sizeof(int) * size);
	m->NUM = (int *) malloc(sizeof(int) * size);

	srand(time(0));

	int i = 0;
	int current_index = 0;
	fscanf(fp, "%d", &bufc);
	while(!feof(fp)) {
		//if blank columns, fill in JC
		while(i < bufc) {
			m->JC[i] = current_index;
			i++;
		}

		m->JC[i] = current_index;
		while(!feof(fp) && i == bufc) {
			fscanf(fp, "%d", &bufr);
			//TODO: check if current_index is out of bounds of array
			m->IR[current_index] = bufr;
			m->NUM[current_index] = rand() % 100; 
			current_index++;
			if(current_index == size) {
				m->IR = (int *)realloc(m->IR, 2*size);
				m->NUM = (int *)realloc(m->NUM, 2*size);
				size = 2*size;
			}
			fscanf(fp, "%d", &bufc);
		}
		i++;
	}
	//fill in the remainder of JC
	while(i <= n) {
		//current_index should now be greater than size of IR/NUM
		m->JC[i] = current_index; 
		i++;
	}

	fclose(fp);
	return;
}

void csc(cs_matrix_t *m, const char *file, int n) {
	m->column = 1;
	//use realloc for resize of the IR and NUM arrays
	//JC array is always size n
	//optionally fill in the NUM with randomized numbers (between 1 and 10 maybe?)

	return;
}