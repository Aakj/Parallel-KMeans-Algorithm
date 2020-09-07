#! /bin/bash
#PBS -N assign
#PBS -q courses
#PBS -l nodes=6:ppn=4
#merge output and error into a single job_name.number_of_job_in_queue.
#PBS -j oe
#export fabric infiniband related variables
export I_MPI_FABRICS=shm:tmi
export I_MPI_DEVICE=rdma:OpenIB-cma
#change directory to where the job has been submitted from
cd $PBS_O_WORKDIR
#source paths
source /opt/software/intel17_update4/initpaths intel64
#sort hostnames
sort $PBS_NODEFILE > hostfile

make clean
make
mpicc -o src src.c -lm

d=(1 2)
for d in "${d[@]}"
do
	for (( p=2 ; p <=12 ; p=p+2 )) 
	do
		for (( i=1 ; i <=5 ; i++ ))
		do
			#arguments: no_of_processes, no_of_clusters, dataset
			if (( $d == 1 ))
			then
				mpirun -machinefile hostfile -np $p ./src $p 40 $d >> data1/output_$p.txt
			else
				mpirun -machinefile hostfile -np $p ./src $p 55 $d >> data2/output_$p.txt
			fi
		done
	done
done

#for dataset 1
#python plotscript.py 1
#for dataset 2
#python plotscript.py 2
echo "COMPLETED."
