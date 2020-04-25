/* 
 * 
 * Jason Yik
 * jyik@usc.edu
 * EE451 Final Project
 *
 * Generation of CSC / CSR data structures for matrices. 
 *
 */ 

typedef struct {
	char column;
	int *JC;
	int *IR;
	int *NUM;

	int nzc; //non zero columns/rows
	int nnz; //number of nonzeroes
} cs_matrix_t;

/*
 * Read edges from file into CSR format in m
 * 
 */
void csr(cs_matrix_t *m, const char *file, int n);

/*
 * Read edges from file into CSC format in m
 * 
 */
void csc(cs_matrix_t *m, const char *file, int nonzeroes);