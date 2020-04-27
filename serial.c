/* 
 * 
 * Jason Yik
 * jyik@usc.edu
 * EE451 Final Project
 *
 * Serial execution of DCSC multiplication and block matrix multiplication.
 *
 */ 

#include "compressed.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int binary_search(int *arr, int len, int target) {
	int left = 0;
	int right = len -1;
	int mid;

	while(left <= right) {
		mid = (left + right)/2;
		if(arr[mid] == target) return mid;
		if(arr[mid] < target) {
			left = mid +1;
			continue;
		}
		if(arr[mid] > target) {
			right = mid -1;
			continue;
		}
	}

	return -1;
}

void dcsc_multiply(int **C, int n, char *Afile, char *Bfile, int Arseed, int Brseed) {
	struct timespec start, stop;
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}
		
	//setup
	cs_matrix_t m1;
	dcs_matrix_t A;
	cs(&m1, Afile, n, 1, Arseed);
	dcs(&m1, &A);

	cs_matrix_t m2;
	dcs_matrix_t B;
	cs(&m2, Bfile, n, 1, Brseed);
	dcs(&m2, &B);	
	
	//multiplication
	for(int x = 0; x < B.nzc; x++) {
		int j = B.JC[x]; // column of C
		int curr = B.CP[x];
		int last = B.CP[x+1];
		while(curr != last) { // iterate over elements in column j of B
			int brow = B.IR[curr];
			int bval = B.NUM[curr];

			int apos = binary_search(A.JC, A.nzc, brow);
			if(apos != -1) { // A contains elements to multiply with this index of B
				int acurr = A.CP[apos];
				int alast = A.CP[apos+1];

				while(acurr != alast) { // iterate over elements in column brow of A
					int i = A.IR[acurr];
					int aval = A.NUM[acurr];

					C[i][j] += aval * bval;

					acurr++;
				}
			}

			curr++;
		}
	}

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}	

	//print time
	double time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("DCSC A nnz: %d, A nzc: %d, B nnz: %d, B nzc: %d\nExecution time: %f\n", A.nnz, A.nzc, B.nnz, B.nzc, time);

	free(A.JC);
	free(A.CP);
	free(A.IR);
	free(A.NUM);

	free(B.JC);
	free(B.CP);
	free(B.IR);
	free(B.NUM);

	return;
}

void read_matrix(int **m, char *file, int seed) {
	FILE *fp = fopen(file, "r");

	int bufc, bufr;
	srand(seed);

	fscanf(fp, "%d", &bufc);
	while(!feof(fp)) {
		fscanf(fp, "%d", &bufr);
		m[bufr][bufc] = rand() % 100;
		fscanf(fp, "%d", &bufc);
	}

	fclose(fp);
	return;
}

void serial_multiply(int **C, int n, char *Afile, char *Bfile, int Arseed, int Brseed) {
	//O(n^3) block multiplication from pa1
	struct timespec start, stop;
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	//read graph into matrices
	int **A = (int **) malloc (sizeof(int *)*n);
	int **B = (int **) malloc (sizeof(int *)*n);
	for (int i=0; i<n; i++) {
		A[i] = (int *) malloc(sizeof(int)*n);
		B[i] = (int *) malloc(sizeof(int)*n);
	}

	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			A[i][j] = 0;
			B[i][j] = 0;
		}
	}

	read_matrix(A, Afile, Arseed);
	read_matrix(B, Bfile, Brseed);

	int b = 16;
	int m = n / b;
	int i, j;
	int k, u, v, w;
	for(i = 0; i < m; i++) {
		for(j = 0; j < m; j++) {
			//iterate over every block in A's row, B's column
			for(k = 0; k < m; k++) {
				//multiply A block by B block
				for(u = 0; u < b; u++) {
					for(v = 0; v < b; v++) {
						for(w = 0; w < b; w++) {
							//C(i, j)(u, v) += A(i, k)(u, w) * B(k, j)(w, v)
							C[i*b + u][j*b + v] = C[i*b + u][j*b + v] + A[i*b + u][k*b + w] * B[k*b + w][j*b + v];
						}
					}
				}
			}
		}
	}

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}	

	//print time
	double time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("Serial Block Multiplication\nExecution time: %f\n", time);

	for (i=0; i<n; i++) {
		free(A[i]);
		free(B[i]);
	}
	free(A);
	free(B);

	return;
}

int main(int argc, char **argv) {
	if(argc < 6) {
		printf("arguments: n Afile Arseed Bfile Brseed\n");
		return 1;
	}
	
	int n = atoi(argv[1]);
	int Arseed = atoi(argv[3]);
	int Brseed = atoi(argv[5]);
	int num_iterations = 5;

	if(n % 1024 != 0) {
		printf("n must be divisible by 1024\n");
		return 1;
	}

	int **C1 = (int **) malloc (sizeof(int *)*n);
	int **C2 = (int **) malloc (sizeof(int *)*n);
	for (int i=0; i<n; i++) {
		C1[i] = (int *) malloc(sizeof(int)*n);
		C2[i] = (int *) malloc(sizeof(int)*n);
	}

	//number of iterations of the program
	for(int it = 0; it < num_iterations; it++) {
		//initialize C
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < n; j++) {
				C1[i][j] = 0;
				C2[i][j] = 0;
			}
		}

		//execute multiplications
		serial_multiply(C1, n, argv[2], argv[4], Arseed, Brseed);
		dcsc_multiply(C2, n, argv[2], argv[4], Arseed, Brseed);

		//check that C is correct here
		if(it == 0) {
			char correct = 1;
			for(int i = 0; i < n; i++) {
				for(int j = 0; j < n; j++) {
					if(C1[i][j] != C2[i][j]) {
						correct = 0;
						printf("Output Matrices differ at [%d][%d]\n", i, j);
					}
				}
			}
			if(correct) printf("Output Matrices do not differ\n");
		}

	}
	

	for (int i=0; i<n; i++) {
		free(C1[i]);
		free(C2[i]);
	}
	free(C1);
	free(C2);

	return 0;
}
