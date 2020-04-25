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

	int n;
	int nzc; // non zero columns/rows
	int nnz; // number of nonzeroes
} dcs_matrix_t;

/*
 * Read edges from file into singly compressed format
 * 
 */
void cs(cs_matrix_t *m, const char *file, int nonzeroes, char column);

/*
 * Convert compressed matrix into doubly compressed matrix
 * 
 */
void dcs(cs_matrix_t *m, dcs_matrix_t *d);
