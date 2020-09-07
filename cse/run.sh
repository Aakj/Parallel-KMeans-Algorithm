#!/bin/bash
make clean
make
mpicc -o src src.c -lm
d=(1 2)
for d in "${d[@]}"
do
	for (( p=2 ; p <= 12; p=p+2 )) 
	do
		#arguments: no_of_processes, process_per_node
		./hosts_avail.sh $p 4
		for (( i=1 ; i <=5 ; i++ ))
		do
			#arguments: no_of_processes, no_of_clusters, dataset
			if (( $d == 1 ))
			then
				mpiexec -ppn 4 -np $p -f hostfile ./src $p 40 $d >> data$d/output_$p.txt
			fi
			if (( $d == 2 ))
			then 
				mpiexec -ppn 4 -np $p -f hostfile ./src $p 55 $d >> data$d/output_$p.txt
			fi
		done
	done
done
#for boxplot of dataset 1
python plotscript.py 1
#for boxplot of dataset 2
python plotscript.py 2



