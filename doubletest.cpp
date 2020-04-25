#include <iostream>
#include "compressed.h"

using namespace std;

int main(int argc, char **argv) {
	cs_matrix_t m;
	if(argc < 2) {
		cerr << "provide n as argument" << endl;
		return 1;
	}
	int n = atoi(argv[1]);
	cs(&m, "graph.txt", n, 1, 1);

	dcs_matrix_t d;
	dcs(&m, &d);

	cout << endl << "JC: ";
	for(int i = 0; i < d.nzc; i++) {
		cout << d.JC[i] << " ";
	}
	cout << endl << endl;
	cout << "CP: ";
	for(int i = 0; i < d.nzc+1; i++) {
		cout << d.CP[i] << " ";
	}
	cout << endl << endl;
	cout << "IR: ";
	for(int i = 0; i< d.nnz; i++) {
		cout << d.IR[i] << " ";
	}
	cout << endl << endl;

	cout << "NUM: ";
	for(int i = 0; i< d.nnz; i++) {
		cout << d.NUM[i] << " ";
	}
	cout << endl << endl;
	

	free(d.JC);
	free(d.CP);
	free(d.IR);
	free(d.NUM);

	return 0;
}