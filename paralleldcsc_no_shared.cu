/* 
 * 
 * Jason Yik
 * jyik@usc.edu
 * EE451 Final Project
 *
 * CUDA implementation of DCSC matrix multiplication.
 * Implementation without shared memory.
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
#include <math.h>

#define GRID_WIDTH 128
#define BLOCK_WIDTH 16
#define BLOCK_HEIGHT 32

//TODO: possibly increase block width, decrease block height since there will be max 8 nonzeroes per col on avg

typedef struct {
	char column;
	int *JC;
	int *IR;
	int *NUM;

	int n;   // number of columns
	int nzc; // non zero columns/rows
	int nnz; // number of nonzeroes
} cs_matrix_t;

typedef struct {
	char column;
	int *JC;
	int *CP;
	int *IR;
	int *NUM;

	int n;   // number of columns
	int nzc; // non zero columns/rows
	int nnz; // number of nonzeroes
} dcs_matrix_t;

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

__global__ void device_multiply(dcs_matrix_t A, dcs_matrix_t B, int *C, int num_cols_per_block, int n) {
	int block_first = blockIdx.x * num_cols_per_block;
	if(block_first > B.nzc) return; // more blocks than nzc
	int block_last = block_first + num_cols_per_block; //exclusive
	if(block_last > B.nzc) block_last = B.nzc;

	//TODO: setup shared memory --> while loop for each thread, once all threads break they can sync

	//loop for the columns that this will look at
	int x = block_first + threadIdx.x; // index in B.JC this thread col is working on
	while(x < block_last) {
		int j = B.JC[x];
		int first = B.CP[x];
		int last = B.CP[x+1];
		int curr = first + threadIdx.y; // row index in B.IR this thread is working on
		//loop for the nonzero elements that this thread will execute on
		while(curr < last) {
			//do the multiplication, remember to atomicAdd for C
			int brow = B.IR[curr];
			int bval = B.NUM[curr];

			int apos = binary_search(A.JC, A.nzc, brow);
			if(apos != -1) {
				int acurr = A.CP[apos];
				int alast = A.CP[apos+1];

				int i, aval;
				while(acurr != alast) { // iterate over elements in column brow of A
					i = A.IR[acurr];
					aval = A.NUM[acurr];

					// C[i * n + j] += aval * bval;
					atomicAdd(C + (i*n + j), aval * bval); // race conditions may occur within this thread row

					acurr++;
				}
			}

			curr += blockDim.y; // next non-zero assigned round robin
		}
		
		x += blockDim.x; // next column is assigned round robin
	}

	//note: threads don't have to wait for each other to sync, some can be on different columns than others no problem
}

void parallel_multiply(int *C, int n, int nnz, char *Afile, char *Bfile, int Arseed, int Brseed) {
	//start timer
	struct timespec start, computation_done;
	double time;
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

	double nd = (double)B.nzc / (double)GRID_WIDTH;
	int num_cols_per_block = (int) ceil(nd);

	device_multiply<<<dimGrid, dimBlock>>>(A, B, C, num_cols_per_block, n);
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

	int *C;
	char Afile[20];
	char Bfile[20];
	cudaMallocManaged(&C, sizeof(int)*n*n);

	//number of iterations of the program
	for(int it = 0; it < num_iterations; it++) {
		//initialize C
		for(int i = 0; i < n*n; i++) {
			C[i] = 0;
		}

		sprintf(Afile, "%d_%d_%d", n, nnz, it);
		sprintf(Bfile, "%d_%d_%d", n, nnz, (it+1) % num_iterations);

		//execute multiplication
		parallel_multiply(C, n, nnz, Afile, Bfile, 1, 1);

		//verify that C is correct here - deleted for sake of execution time
	}

	//TODO: since submitting CUDA takes so long on HPC, probabaly want to write batch
	// or run all of the tests in the same executable sequence, make changes after the
	// algorithm is shown to work
	
	cudaFree(C);

	return 0;
}
