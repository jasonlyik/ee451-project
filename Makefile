serial: serialdcsc.c compressed.h compressed.c
	gcc serialdcsc.c compressed.c -o serial

parallel: paralleldcsc.cu compressed.h compressed.c
	source /usr/usc/cuda/default/setup.sh
	nvcc paralleldcsc.cu compressed.c -o parallel

data_structure_tests: singletest.cpp doubletest.cpp compressed.c compressed.h
	g++ singletest.cpp compressed.c -o singletest
	g++ doubletest.cpp compressed.c -o doubletest

clean:
	rm serial singletest doubletest