#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "compressed.h"

// See output file of rmat.py for assumed input format

void cs(cs_matrix_t *m, const char *file, int n, char column, int random_seed) {
	m->column = column;
	m->n = n;

	FILE *fp = fopen(file, "r");
	int bufc, bufr;

	int size = 1024;
	m->JC = (int *) malloc(sizeof(int) * (n+1));
	m->IR = (int *) malloc(sizeof(int) * size);
	m->NUM = (int *) malloc(sizeof(int) * size);

	srand(random_seed);
	//srand(time(0));

	int nzc = 0;

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
		nzc++;
		while(!feof(fp) && i == bufc) {
			fscanf(fp, "%d", &bufr);
			m->IR[current_index] = bufr;
			m->NUM[current_index] = rand() % 100; 
			current_index++;
			if(current_index == size) {
				m->IR = (int *)realloc(m->IR, sizeof(int) * (2*size));
				m->NUM = (int *)realloc(m->NUM, sizeof(int) * (2*size));
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

	m->nnz = current_index;
	m->nzc = nzc;

	fclose(fp);
	return;
}

void dcs(cs_matrix_t *m, dcs_matrix_t *d) {
	d->column = m->column;
	d->IR = m->IR;
	d->NUM = m->NUM;

	d->n = m->n;
	d->nnz = m->nnz;
	d->nzc = m->nzc;

	d->JC = (int *) malloc(sizeof(int) * m->nzc);
	d->CP = (int *) malloc(sizeof(int) * (m->nzc +1));

	int current_index = 0;
	for(int i = 0; i < m->n; i++) {
		if(m->JC[i] == m->JC[i+1]) {
			continue;
		}
		else {
			d->JC[current_index] = i;
			d->CP[current_index] = m->JC[i];
			current_index++;
		}
	}
	d->CP[current_index] = m->nnz;

	//invalidate m
	free(m->JC);

	return;
}