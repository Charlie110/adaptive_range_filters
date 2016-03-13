import matplotlib.pyplot as plt
from matplotlib.path import Path
import matplotlib.patches as patches
import sys
import os
import shutil


if __name__ == "__main__":


	#all_data = [[0,0],[1,0],[0,1],[1,1],[2,0]]#,[3,0],[2,1],[3,1],[0,2],[1,2]]

	#for i in range(0,len(all_data)-1):
	#	pyplot.plot([all_data[i][1], all_data[i][0]], [all_data[i+1][1], all_data[i+1][0]],'go-')
	#	print "plotting:",all_data[i],"to",all_data[i+1]


	useLines = 1
	filename = str(sys.argv[1])
	if(len(sys.argv)>2):
		useLines = int(sys.argv[2])
	f = open(filename,'r')
	verts =[]
	colors = []
	labels = []
	ptx = []
	pty = []

	firstline = f.readline().split()
	

	if(int(firstline[0])!=2):
		print "The space filling curve is not two dimensional"
		sys.exit()
	else:
		xlim = int(firstline[1])
		ylim = int(firstline[2])

	i = 0
	for line in f:
		l = line.split()
		coords = (int(l[0]),int(l[1]))
		#print coords
		verts.append(coords)
		val = int(l[2])
		ptx.append(int(l[0]))
		pty.append(int(l[1]))
		if(val == 1):
			colors.append('green')
		else:
			colors.append('tomato')
		labels.append(i)
		i = i + 1
		
	
	f.close()

	codes = [Path.MOVETO];
	for i in range(0,len(verts)-1):
		codes.append(Path.LINETO)
	

	path = Path(verts, codes)
	fig = plt.figure()
	ax = fig.add_subplot(111)
	if(useLines == 1):
		patch = patches.PathPatch(path, facecolor='none', lw=1)
		ax.add_patch(patch)
	ax.set_xlim(-0.1,xlim)
	ax.set_ylim(-0.1,ylim)
	ax.scatter(ptx,pty,s=20,color=colors);

	i = 0
	for v in verts:
		ax.annotate(i, xy=(v[0], v[1]),  xycoords='data',
                xytext=(10, 15), textcoords='offset points',
                arrowprops=dict(arrowstyle="->")
                )
		i =  i + 1
	#ax.set_ylim(ax.get_ylim()[::-1]) # puts origin in upper left 
	plt.show()

