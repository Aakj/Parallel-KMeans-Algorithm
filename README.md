## Parallely clustering moving particles using MPI

- Given: The input data (3D coordinates) is contained in binary files corresponding to a time step. 
- Task: To cluster moving particles in a 3D space.
- Approach: Parallelized the K-means clustering algorithm by distributing the particles uniformly on assigned nodes in CSE cluster or HPC followed by gathering the relevant information on a node. The process is repeated till convergence is obtained.
- This was done as a part of course CS633 instructed by Prof. Preeti Malakar.

