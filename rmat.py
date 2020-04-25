import snap
import time

now = time.time()
Rnd = snap.TRnd()
Rnd.PutSeed(int(now))

#dimensions: n = 2^18, e = 2^16 ??
#todo, fiddle with numbers, commonly see a:b = a:c = 3:1 approx
Graph = snap.GenRMat(1000, 100, .5, .17, .17, Rnd) 
snap.PrintInfo(Graph, "RMat Graph", "graph_info.txt", False)

# save as binary file, not useful for me
# FOut = snap.TFOut("graph.txt")
# Graph.Save(FOut)
# FOut.Flush()

f = open("graph.txt", "w+")
#f.write("Non-Zeroes of Matrix\n")
#f.write("Row        Col\n")

for EI in Graph.Edges():
	src = '{:10}'.format("%d" % (EI.GetSrcNId()))
	dst = '{:10}'.format("%d" % (EI.GetDstNId()))
	f.write(src + " " + dst + "\n")

f.flush()