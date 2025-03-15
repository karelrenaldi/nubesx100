# IF3230-K04-nubesx100

Parallel and Distributed Systems - Matrix Convolution

## Parallelization Scheme

### OpenMP

To compute the convolution matrix values, the for-loop over the rows and columns of the output matrix is flattened into a single array loop. Then, a parallel for-loop is executed with `num_threads` set to `thread_count`. To find the maximum range in the convolution matrix, the matrix is flattened again into an array. Each element is then compared: if it is greater than the current max, the max value is updated in a critical section; if it is smaller than the current min, the min value is updated similarly. Once all elements are processed, the final range is computed as `max - min`.

### OpenMPI

The kernel matrix and the number of threads used are broadcasted to all processes. The matrix data is then scattered among the processes, allowing each process to allocate an appropriate array size. Using this size, a scatterv operation distributes the matrix data evenly across processes. Each process then allocates an array to store the calculated range using OpenMP. This array is locally sorted before being gathered back to the root process (rank 0) using a gatherv operation. At process 0, the collected results are sorted again to generate the final output used for statistical analysis.

## Best Execution Analysis

1. **Parallelization Strategy**  
   Parallelization is achieved using OpenMPI for distributing matrices across processes and OpenMP for distributing workloads across threads. The parallelization scheme follows the approach described earlier.

2. **Best Execution Time**  
   For TC2, TC3, and TC4, parallel execution completes faster. However, the best speedup is achieved for TC4, which processes 5000 matrices.

## Comparison of Serial and Parallel Execution

Execution results are as follows:

```shell
TC1 => serial
TC2 => parallel
TC3 => parallel
TC4 => parallel
```

From this data, it is evident that increasing the number and size of matrices improves parallel performance. Consequently, for small test cases, the parallel program does not significantly outperform the serial program. However, for larger test cases, the parallel program is significantly more efficient.

## Execution Variations and the Impact of OpenMP and OpenMPI

```shell
| TC  | OpenMPI Nodes | OpenMP Threads | Execution Time (seconds) |
| --- | ------------ | ------------- | ----------------------- |
| 1   | 2            | 5             | 0.027230                |
| 1   | 2            | 16            | 0.030819                |
| 1   | 3            | 5             | 0.043672                |
| 1   | 3            | 16            | 0.029709                |
| 1   | 4            | 5             | 0.036623                |
| 1   | 4            | 16            | 0.037708                |
| 1S  | -            | -             | 0.008659                |
| 2   | 2            | 5             | 0.611569                |
| 2   | 2            | 16            | 0.625597                |
| 2   | 3            | 5             | 0.538054                |
| 2   | 3            | 16            | 0.525512                |
| 2   | 4            | 5             | 0.559031                |
| 2   | 4            | 16            | 0.494060                |
| 2S  | -            | -             | 0.698502                |
| 3   | 2            | 5             | 0.789968                |
| 3   | 2            | 16            | 0.686185                |
| 3   | 3            | 5             | 0.785868                |
| 3   | 3            | 16            | 1.368298                |
| 3   | 4            | 5             | 1.570319                |
| 3   | 4            | 16            | 0.835112                |
| 3S  | -            | -             | 0.677666                |
| 4   | 2            | 5             | 8.602319                |
| 4   | 2            | 16            | 8.239505                |
| 4   | 3            | 5             | 8.936908                |
| 4   | 3            | 16            | 10.688829               |
| 4   | 4            | 5             | 13.540155               |
| 4   | 4            | 16            | 9.101202                |
| 4S  | -            | -             | 13.078884               |
```

**S denotes serial execution time.**

The table above shows execution time variations for different numbers of OpenMPI nodes and OpenMP threads in each test case. Generally, for the same number of nodes, increasing the number of threads tends to slow down execution due to increased overhead from thread synchronization, especially when accessing shared variables or data. A similar effect occurs when increasing the number of nodes for the same number of threads, leading to greater overhead due to process blocking in various execution phases.

However, as data size increases, processing speed significantly improves, and parallel execution time becomes much better. The table shows that for Test Case 1 (small data size), the parallel program performs worse than the serial one, whereas for larger test cases, parallel execution significantly outperforms the serial approach.

## Authors

1. **13519180** Karel Renaldi  
2. **13519185** Richard Rivaldo  
3. **13519205** Muhammad Rifat Abiwardani
