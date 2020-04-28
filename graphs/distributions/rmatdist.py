#
# Jason Yik
# jyik@usc.edu
# EE451 Final Project
#
# Generation of randomized edges with different distributions for graph
#

import sys
import snap
import time

def main():
	if len(sys.argv) < 4:
		print("enter a b c as arguments")
		return

	n = 32768
	e = 262144

	a = float(sys.argv[1])
	b = float(sys.argv[2])
	c = float(sys.argv[3])

	now = time.time()
	Rnd = snap.TRnd()
	Rnd.PutSeed(int(now))

	for i in range(5):
		Graph = snap.GenRMat(n, e, a/100, b/100, c/100, Rnd) 

		filestr = str(int(a)) + "_" + str(int(b)) + "_" + str(int(c)) + "_" + str(i)

		f = open(filestr, "w+")

		for EI in Graph.Edges():
			src = '{:10}'.format("%d" % (EI.GetSrcNId()))
			dst = '{:10}'.format("%d" % (EI.GetDstNId()))
			f.write(src + " " + dst + "\n")

		f.flush()

if __name__ == '__main__':
	main()