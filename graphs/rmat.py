#
# Jason Yik
# jyik@usc.edu
# EE451 Final Project
#
# Generation of randomized edges for graph
#

import sys
import snap
import time

def main():
	if len(sys.argv) < 3:
		print("enter n and nnz as arguments")
		return

	n = int(sys.argv[1])
	e = int(sys.argv[2])

	now = time.time()
	Rnd = snap.TRnd()
	Rnd.PutSeed(int(now))

	for i in range(5):
		Graph = snap.GenRMat(n, e, .25, .25, .25, Rnd) 

		filestr = str(n) + "_" + str(e) + "_" + str(i)

		f = open(filestr, "w+")

		for EI in Graph.Edges():
			src = '{:10}'.format("%d" % (EI.GetSrcNId()))
			dst = '{:10}'.format("%d" % (EI.GetDstNId()))
			f.write(src + " " + dst + "\n")

		f.flush()

if __name__ == '__main__':
	main()