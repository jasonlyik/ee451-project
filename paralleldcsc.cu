/* 
 * 
 * Jason Yik
 * jyik@usc.edu
 * EE451 Final Project
 *
 * CUDA implementation of DCSC matrix multiplication.
 * 
 * Run on USC HPC:
 * srun -n1 --gres=gpu:1 ./parallel <n> <nnz>
 *
 * Run with executable in a graphs folder containing n_nnz_x graphs
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cublas.h>

#define GRID_WIDTH 128
#define BLOCK_WIDTH 16
#define BLOCK_HEIGHT 32

//TODO: delete this
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

//TODO: delete this
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

//uses CUDA unified memory
void cuda_cs(cs_matrix_t *m, const char *file, int n, int nnz, char column, int random_seed) {
	m->column = column;
	m->n = n;

	FILE *fp = fopen(file, "r");
	int bufc, bufr;

	cudaMallocManaged(&(m->JC), sizeof(int) * (n+1));
	cudaMallocManaged(&(m->IR), sizeof(int) * nnz);
	cudaMallocManaged(&(m->NUM), sizeof(int) * nnz);
	
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

//uses CUDA unified memory
void cuda_dcs(cs_matrix_t *m, dcs_matrix_t *d) {
	d->column = m->column;
	d->IR = m->IR;
	d->NUM = m->NUM;

	d->n = m->n;
	d->nnz = m->nnz;
	d->nzc = m->nzc;

	cudaMallocManaged(&(d->JC), sizeof(int) * m->nzc);
	cudaMallocManaged(&(d->CP), sizeof(int) * (m->nzc +1));

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
	cudaFree(m->JC);

	return;
}

__device__ int binary_search(int *arr, int len, int target) {
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

__global__ void device_multiply(dcs_matrix_t A, dcs_matrix_t B, int *C, int n) {
	//TODO: write this
	
}

void parallel_multiply(int *C, int n, int nnz, char *Afile, char *Bfile, int Arseed, int Brseed) {
	//start timer
	struct timespec start, computation_done;
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

	//setup
	cs_matrix_t m1;
	dcs_matrix_t A;
	cuda_cs(&m1, Afile, n, nnz, 1, Arseed);
	cuda_dcs(&m1, &A);

	cs_matrix_t m2;
	dcs_matrix_t B;
	cuda_cs(&m2, Bfile, n, nnz, 1, Brseed);
	cuda_dcs(&m2, &B);

	//call device multiply
	dim3 dimGrid(GRID_WIDTH);
	dim3 dimBlock(BLOCK_WIDTH, BLOCK_HEIGHT);

	device_multiply<<<dimGrid, dimBlock>>>(A, B, C, n);
	cudaDeviceSynchronize(); // in order to access unified memory

	//stop timer for computation
	if( clock_gettime(CLOCK_REALTIME, &computation_done) == -1) { perror("clock gettime");}

	//print out time for finishing computation and copying back data
	time = (computation_done.tv_sec - start.tv_sec)+ (double)(computation_done.tv_nsec - start.tv_nsec)/1e9;
	printf("DCSC A nnz: %d, A nzc: %d, B nnz: %d, B nzc: %d\nExecution Time: %f\n", A.nnz, A.nzc, B.nnz, B.nzc, time);

	//free unified memory
	cudaFree(A.JC);
	cudaFree(A.CP);
	cudaFree(A.IR);
	cudaFree(A.NUM);

	cudaFree(B.JC);
	cudaFree(B.CP);
	cudaFree(B.IR);
	cudaFree(B.NUM);
}

int main(int argc, char **argv) {
	if(argc < 3) {
		printf("arguments: n nnz\n");
		return 1;
	}
	
	int n = atoi(argv[1]);
	int nnz = atoi(argv[2]);
	int num_iterations = 5;

	cudaMallocManaged(&C, sizeof(int)*n*n);

	//number of iterations of the program
	for(int it = 0; it < num_iterations; it++) {
		//initialize C
		for(int i = 0; i < n^2; i++) {
			C[i] = 0;
		}

		sprintf(Afile, "%d_%d_%d", n, nnz, it);
		sprintf(Bfile, "%d_%d_%d", n, nnz, (it+1) % num_iterations);

		//execute multiplication
		parallel_multiply(C, n, nnz, Afile, Bfile, 1, 1);

		//verify that C is correct here
		//TODO: get rid of this
		if(it == 0) {
			int **C2 = (int **) malloc (sizeof(int *)*n);
			for (int i=0; i<n; i++) {
				C2[i] = (int *) malloc(sizeof(int)*n);
			}
			for(int i = 0; i < n; i++) {
				for(int j = 0; j < n; j++) {
					C2[i][j] = 0;
				}
			}
			serial_multiply(C2, n, Afile, Bfile, 1, 1);

			char correct = 1;
			for(int i = 0; i < n; i++) {
				for(int j = 0; j < n; j++) {
					if(C[i*n + j] != C2[i][j]) {
						correct = 0;
						printf("Output Matrices differ at [%d][%d]\n", i, j);
					}
				}
			}
			if(correct) printf("Output Matrices do not differ\n");

			for (int i=0; i<n; i++) {
				free(C2[i]);
			}
			free(C2);
		}

	}
	
	cudaFree(C);

	return 0;
}