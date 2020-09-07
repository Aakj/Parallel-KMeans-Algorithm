#!/usr/bin/env python
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as mpl
import sys

dataset = sys.argv[1]
data=np.loadtxt('databox'+str(dataset)+'.txt',delimiter=' ')
p0 = []
p1 = []
p2 = []
p3 = []
p4 = []
p5 = []
color=['g','r','c']
naming=['Pre-pro.','Proc.','Total']
index=0
#segregating data
for p in range(3):
    for d in range(6):
        t=[]
        for i in range(15):
            if (i+d*15)%3==p:
                t.append(data[i+d*15])
        if d==0:
            p0=t
        elif d==1:
            p1=t
        elif d==2:
            p2=t
        elif d==3:
            p3=t
        elif d==4:
            p4=t
        elif d==5:
            p5=t
    #for one amongst the 3 plots
    boxplot_data=[p0,p1,p2,p3,p4,p5]
    mpl.boxplot(boxplot_data,patch_artist=True,labels=['2','4','6','8','10','12'])
    #for finding median
    p0.sort()
    p1.sort()
    p2.sort()
    p3.sort()
    p4.sort()
    p5.sort()
    #drawing lines
    mpl.plot([1,2],[p0[2],p1[2]],color[index],label=naming[index])
    mpl.plot([2,3],[p1[2],p2[2]],color[index])
    mpl.plot([3,4],[p2[2],p3[2]],color[index])
    mpl.plot([4,5],[p3[2],p4[2]],color[index])
    mpl.plot([5,6],[p4[2],p5[2]],color[index])
    index = index + 1

#making boxplot
mpl.title('Boxplot for Dataset '+str(dataset))
mpl.xlabel('No. of Processes')
mpl.ylabel('Time (seconds)')
mpl.legend(bbox_to_anchor=(1, 1), loc=1, borderaxespad=0.,fontsize=7)
mpl.savefig('data'+str(dataset)+'/plot.png')
mpl.clf()
