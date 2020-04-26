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
	//TODO here
	//TODO test this in a different file with small matrices
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

int main(int argc, char **argv) {
	if(argc < 6) {
		printf("arguments: n Afile Arseed Bfile Brseed\n");
		return 1;
	}
	
	int n = atoi(argv[1]);
	int Arseed = atoi(argv[3]);
	int Brseed = atoi(argv[5]);
	int num_iterations = 5;

	int **C1 = (int **) malloc (sizeof(int *)*n);
	for (int i=0; i<n; i++) {
		C1[i] = (int *) malloc(sizeof(int)*n);
	}

	//number of iterations of the program
	for(int it = 0; it < num_iterations; it++) {
		//initialize C
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < n; j++) {
				C1[i][j] = 0;
			}
		}

		//execute multiplications
		dcsc_multiply(C1, n, argv[2], argv[4], Arseed, Brseed);

		//check that C is correct here
		if(it == 0) {
			for(int i = 0; i < n; i++) {
				for(int j = 0; j < n; j++) {
					printf("%d ", C1[i][j]); 
				}
				printf("\n");
			}
		}

	}
	

	for (int i=0; i<n; i++) {
		free(C1[i]);
	}
	free(C1);

	return 0;
}