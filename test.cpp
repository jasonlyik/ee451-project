#include <iostream>
#include "single_compressed.h"

using namespace std;

int main(int argc, char **argv) {
	cs_matrix_t m;
	if(argc < 3) {
		cerr << "provide n and nnz as arguments" << endl;
		return 1;
	}
	int n = atoi(argv[1]);
	int nnz = atoi(argv[2]);
	csr(&m, "graph.txt", n);

	cout << endl << "JC: ";
	for(int i = 0; i < n+1; i++) {
		cout << m.JC[i] << " ";
	}
	cout << endl << endl;
	cout << "IR: ";
	for(int i = 0; i< nnz; i++) {
		cout << m.IR[i] << " ";
	}
	cout << endl << endl;

	cout << "NUM: ";
	for(int i = 0; i< nnz; i++) {
		cout << m.NUM[i] << " ";
	}
	cout << endl << endl;

	free(m.JC);
	free(m.IR);
	free(m.NUM);

	return 0;
}