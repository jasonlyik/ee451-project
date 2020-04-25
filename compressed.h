/* 
 * 
 * Jason Yik
 * jyik@usc.edu
 * EE451 Final Project
 *
 * Generation of compressed data structures for matrices. 
 *
 */ 

typedef struct {
	char column;
	int *JC;
	int *IR;
	int *NUM;

	int nzc; // non zero columns/rows
	int nnz; // number of nonzeroes
} cs_matrix_t;

typedef struct {
	char column;
	int *JC;
	int *CP;
	int *IR;
	int *NUM;

	int nzc; // non zero columns/rows
	int nnz; // number of nonzeroes
} dcs_matrix_t;

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

/*
 * Convert CSR matrix into DCSR
 * 
 */
void dcsr(cs_matrix_t *m, dcs_matrix_t *d);

/*
 * Convert CSC matrix into DCSC
 * 
 */
void dcsc(cs_matrix_t *m, dcs_matrix_t *d);