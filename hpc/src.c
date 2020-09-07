#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "mpi.h"

double euclideanDist(double x1,double y1,double z1,double x2,double y2,double z2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));
}	

int main(int argc,char *argv[])
{
	int no_of_process = atoi(argv[1]);
	int no_of_clusters = atoi(argv[2]);
	int data_set = atoi(argv[3]);
	int no_of_timestamps;
	char filename[20] = "datain";
	char write_file[20] ="databox";
	char index[5];
	double *time_sum1,*time_sum2,time_total;
	int i,j,k;

	//Variables set according to different dataset
	if( data_set == 1 )
	{
		no_of_timestamps = 17;
		strcat(filename,"1/file");
		strcat(write_file,"1.txt");
		time_sum1 = (double*)malloc(sizeof(double)*17);
		time_sum2 = (double*)malloc(sizeof(double)*17);
	}
	else if( data_set == 2 )
	{
		no_of_timestamps = 16;
		strcat(filename,"2/file");
		strcat(write_file,"2.txt");
		time_sum1 = (double*)malloc(sizeof(double)*16);
		time_sum2 = (double*)malloc(sizeof(double)*16);
	}
	int my_rank,size;
	double time1,time2,timet1,timet2,time_diff;

	//Initialization
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	
	//For Total time
	timet1 = MPI_Wtime(); 	

	//Data distribution
	if(my_rank==0)
		printf("------\nNumber of processes:%d\n",no_of_process);
	for(i=0; i<no_of_timestamps; i++)
	{
		//Opening the required file
		if(my_rank==0)
		{
			printf("T%d: K=%d",i,no_of_clusters);
		}
                int no_of_points,points_per_process,no_of_doubles,divisible; 
		MPI_File filehandle;
		MPI_Status status;                                                                 MPI_Offset filesize;
		sprintf(index,"%d",i);
		char current_file[20];
		strcpy(current_file,filename);			
		strcat(current_file,index);
		MPI_File_open(MPI_COMM_WORLD,current_file,MPI_MODE_RDONLY,MPI_INFO_NULL,&filehandle);
		MPI_File_get_size(filehandle,&filesize);
	        no_of_doubles = filesize/8;
		no_of_points = filesize/32;
		points_per_process = no_of_points/no_of_process;
		divisible = no_of_points % no_of_process;
		double *x_coord = (double*)malloc(sizeof(double)*no_of_points);
                double *y_coord = (double*)malloc(sizeof(double)*no_of_points);
                double *z_coord = (double*)malloc(sizeof(double)*no_of_points);
                double *data = (double*)malloc(sizeof(double)*no_of_doubles);
		int *send_count = (int*)malloc(sizeof(int)*no_of_process);                         int *displ = (int*)malloc(sizeof(int)*no_of_process);

		//Pre-processing begins (Read data files and distributing particles)
		time1 = MPI_Wtime();
		if(my_rank == 0)
		{ 
			//Reading file
			MPI_File_read(filehandle,data,no_of_doubles,MPI_DOUBLE,&status);
		        k = -1;	
			for(j=0;j<no_of_points;j++)
			{
				k++;
				x_coord[j] = data[++k];
				y_coord[j] = data[++k];
				z_coord[j] = data[++k];
			}
		}	
		MPI_File_close(&filehandle);

		//Distributing data
		displ[0]=0;
		for(j=0;j<no_of_process-1;j++)
		{
			send_count[j]=points_per_process;
			displ[j+1]=displ[j]+points_per_process;
		}
		if(divisible)
		{
			send_count[no_of_process-1]=points_per_process+divisible;	
		}
		else
		{
			send_count[no_of_process-1]=points_per_process;
		}

		double *x_coord_spec = (double*)malloc(sizeof(double)*send_count[my_rank]);
		double *y_coord_spec = (double*)malloc(sizeof(double)*send_count[my_rank]);
		double *z_coord_spec = (double*)malloc(sizeof(double)*send_count[my_rank]);
		MPI_Scatterv(x_coord,send_count,displ,MPI_DOUBLE,x_coord_spec,send_count[my_rank],MPI_DOUBLE,0,MPI_COMM_WORLD);
		MPI_Scatterv(y_coord,send_count,displ,MPI_DOUBLE,y_coord_spec,send_count[my_rank],MPI_DOUBLE,0,MPI_COMM_WORLD);
		MPI_Scatterv(z_coord,send_count,displ,MPI_DOUBLE,z_coord_spec,send_count[my_rank],MPI_DOUBLE,0,MPI_COMM_WORLD);
		time2 = MPI_Wtime();
		//Pre-processing ends
		
		time_diff = time2 - time1;
		MPI_Reduce(&time_diff,&time_sum1[i],1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
		
		//Processing begins (K-means Clustering)
		time1 = MPI_Wtime();
		double *x_coord_clus = (double*)malloc(sizeof(double)*no_of_clusters);                                                                                                double *y_coord_clus = (double*)malloc(sizeof(double)*no_of_clusters);
                double *z_coord_clus = (double*)malloc(sizeof(double)*no_of_clusters);
		if(my_rank==0)
		{	
			//Initial Clusters
			k=0;
			int random;
			for(j=0;j<no_of_clusters;j++)
			{
				random = rand()%no_of_points;
				x_coord_clus[k] = x_coord[random];
				y_coord_clus[k] = y_coord[random]; 
				z_coord_clus[k] = z_coord[random];
			        k++;	
			}
		}

		int times=0;
		int *clus_assign = (int*)malloc(sizeof(int)*send_count[my_rank]);
		int *points_per_clus_p = (int*)calloc(no_of_clusters,sizeof(int));
		int *total_points_in_clus = (int*)calloc(no_of_clusters,sizeof(int)); 
		int *clus_assign_to_point = (int*)malloc(sizeof(int)*no_of_points);
		while(times<=100)	
		{
			MPI_Bcast(x_coord_clus,no_of_clusters,MPI_DOUBLE,0,MPI_COMM_WORLD);
			MPI_Bcast(y_coord_clus,no_of_clusters,MPI_DOUBLE,0,MPI_COMM_WORLD); 
			MPI_Bcast(z_coord_clus,no_of_clusters,MPI_DOUBLE,0,MPI_COMM_WORLD);
			
			//Finding cluster for each point
			long p_change_in_clus=0,total_p_change_in_clus=0;
			if(times == 0)
			{
				for(j=0;j<send_count[my_rank];j++)
				{
					clus_assign[j]=-1;
				}
			}
			for(j=0;j<send_count[my_rank];j++)
			{
				double min_d = DBL_MAX;
				int clus_a = -1;
				for(k=0;k<no_of_clusters;k++)
				{
					double d = euclideanDist(x_coord_spec[j],y_coord_spec[j],z_coord_spec[j],x_coord_clus[k],y_coord_clus[k],z_coord_clus[k]);	
					if(d<min_d)
					{
						min_d = d;
						clus_a = k;
					}
				}
				if(times==0)
				{
					if(clus_assign[j]!=clus_a)
					{
						p_change_in_clus++;	
						clus_assign[j]=clus_a;
						points_per_clus_p[clus_a]++;
					}
				}
				else
				{
					if(clus_assign[j]!=clus_a)
					{
						p_change_in_clus++;
						points_per_clus_p[clus_assign[j]]--;
						clus_assign[j]=clus_a;
						points_per_clus_p[clus_a]++;
					}
				}
			}

			MPI_Barrier(MPI_COMM_WORLD);

			//Calculating the no of points in each cluster
			for(j=0;j<no_of_clusters;j++)
			{	
			MPI_Reduce(&points_per_clus_p[j],&total_points_in_clus[j],1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
			}

			//Gathering the cluster id of all points to which they belong
			MPI_Gatherv(clus_assign,send_count[my_rank],MPI_INT,clus_assign_to_point,send_count,displ,MPI_INT,0,MPI_COMM_WORLD);
			
			//Deciding whether to stop the algorithm or not (Convergence point)
			MPI_Allreduce(&p_change_in_clus,&total_p_change_in_clus,1,MPI_LONG,MPI_SUM,MPI_COMM_WORLD);                                                                           
			MPI_Barrier(MPI_COMM_WORLD); 

			if((0.005)*no_of_points>total_p_change_in_clus)
			{	
				break;
			}

			//Updating the Cluster Mean
			if(my_rank==0)
			{
				double x_coord_mean,y_coord_mean,z_coord_mean;
				for(j=0;j<no_of_clusters;j++)
				{
					x_coord_mean = 0;
					y_coord_mean = 0;
					z_coord_mean = 0;
					int count=0;
					for(k=0;k<no_of_points;k++)
					{
						if(clus_assign_to_point[k]==j)
						{
							count++;
							x_coord_mean += x_coord[k];
							y_coord_mean += y_coord[k];							   z_coord_mean += z_coord[k];
							if(count == total_points_in_clus[j])
							{
								break;
							}
						}
					}
					x_coord_clus[j] = x_coord_mean/total_points_in_clus[j];
					y_coord_clus[j] = y_coord_mean/total_points_in_clus[j];                               
					z_coord_clus[j] = z_coord_mean/total_points_in_clus[j];                               
				}
			}

		        times++;		
		}
		time2 = MPI_Wtime();
		//Processing ends

		time_diff = time2 - time1;
		MPI_Reduce(&time_diff,&time_sum2[i],1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);		
		
		MPI_Barrier(MPI_COMM_WORLD);

		//Writing data to Output file
		if(my_rank==0)
		{
			for(j=0;j<no_of_clusters;j++)
			{
				printf(",<%d,(%lf,%lf,%lf)>",total_points_in_clus[j],x_coord_clus[j],y_coord_clus[j],z_coord_clus[j]);
			}
			printf("\n");
		}
	}

	//For Total time
	timet2 = MPI_Wtime();
	time_diff = timet2-timet1;
	MPI_Reduce(&time_diff,&time_total,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

	if(my_rank==0)
	{
		double sum1=0,sum2=0;
		for(i=0;i<no_of_timestamps;i++)
		{
			sum1 = sum1 + time_sum1[i];
			sum2 = sum2 + time_sum2[i];
		}
		double ppt = sum1/(no_of_timestamps*no_of_process);
		double pt = sum2/(no_of_timestamps*no_of_process);
		//Writing to separate file for boxplot
		FILE *file;
		file = fopen(write_file,"a+");
		fprintf(file,"%lf\n",ppt);
		fprintf(file,"%lf\n",pt);
		fprintf(file,"%lf\n",time_total);
		fclose(file);
		printf("\nAverage time to pre-process: %lf\nAverage time to process: %lf\nTotal time: %lf\n------\n",ppt,pt,time_total);
	}
	
	MPI_Finalize();
	return 0;
}
