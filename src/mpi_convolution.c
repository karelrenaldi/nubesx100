#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

#define nil NULL
#define NMAX 100
#define DATAMAX 1000
#define DATAMIN -1000
#define ROOT_RANK 0

typedef struct Matrix
{
    int mat[NMAX][NMAX]; // Matrix cells
    int row_eff;         // Matrix effective row
    int col_eff;         // Matrix effective column
} Matrix;

void init_matrix(Matrix *m, int nrow, int ncol)
{
    m->row_eff = nrow;
    m->col_eff = ncol;

    for (int i = 0; i < nrow; i++)
    {
        for (int j = 0; j < ncol; j++)
        {
            m->mat[i][j] = 0;
        }
    }
}

Matrix input_matrix(int nrow, int ncol)
{
    Matrix input;
    init_matrix(&input, nrow, ncol);

    for (int i = 0; i < nrow; i++)
    {
        for (int j = 0; j < ncol; j++)
        {
            scanf("%d", &input.mat[i][j]);
        }
    }

    return input;
}

void print_matrix(Matrix *m)
{
    for (int i = 0; i < m->row_eff; i++)
    {
        for (int j = 0; j < m->col_eff; j++)
        {
            printf("%d ", m->mat[i][j]);
        }
        printf("\n");
    }
}

void merge_array(int *n, int left, int mid, int right)
{
    int n_left = mid - left + 1;
    int n_right = right - mid;
    int iter_left = 0, iter_right = 0, iter_merged = left;
    int arr_left[n_left], arr_right[n_right];

    for (int i = 0; i < n_left; i++)
    {
        arr_left[i] = n[i + left];
    }

    for (int i = 0; i < n_right; i++)
    {
        arr_right[i] = n[i + mid + 1];
    }

    while (iter_left < n_left && iter_right < n_right)
    {
        if (arr_left[iter_left] <= arr_right[iter_right])
        {
            n[iter_merged] = arr_left[iter_left++];
        }
        else
        {
            n[iter_merged] = arr_right[iter_right++];
        }
        iter_merged++;
    }

    while (iter_left < n_left)
    {
        n[iter_merged++] = arr_left[iter_left++];
    }
    while (iter_right < n_right)
    {
        n[iter_merged++] = arr_right[iter_right++];
    }
}

void merge_sort(int *n, int left, int right)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;

        merge_sort(n, left, mid);
        merge_sort(n, mid + 1, right);

        merge_array(n, left, mid, right);
    }
}

int integer_ceil(int a, int b)
{
    return a / b + (a % b != 0);
}

int openmp(int rank)
{
    srand(time(nil) + rank);
    return rand();
}

int get_median(int *n, int length)
{
    int mid = length / 2;
    if (length & 1)
        return n[mid];

    return (n[mid - 1] + n[mid]) / 2;
}

long get_floored_mean(int *n, int length)
{
    long sum = 0;
    for (int i = 0; i < length; i++)
    {
        sum += n[i];
    }

    return sum / length;
}

int main(int argc, char **argv)
{
    double start_time, end_time, duration, runtime;
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        perror("Error initializing MPI\n");
        exit(1);
    }
    start_time = MPI_Wtime();

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    Matrix *arr_m;
    Matrix kernel;
    int num_targets;
    int *global_results_array;
    int *local_mat_sizes = malloc(size * sizeof(int));
    int *sendcount = malloc(size * (sizeof(int)));
    int *displs = malloc(size * (sizeof(int)));

    if (rank == ROOT_RANK)
    {
        // Scan users' input
        int kernel_row, kernel_col, target_row, target_col;
        scanf("%d %d", &kernel_row, &kernel_col);
        kernel = input_matrix(kernel_row, kernel_col);

        scanf("%d %d %d", &num_targets, &target_row, &target_col);
        if ((arr_m = malloc(num_targets * sizeof(Matrix))) == nil)
        {
            printf("Malloc error arr_m\n");
            exit(1);
        }

        for (int i = 0; i < num_targets; i++)
        {
            arr_m[i] = input_matrix(target_row, target_col);
        }

        // Map Scatterv arguments
        int remaining_matrices = num_targets;
        for (int i = 0; i < size; i++)
        {
            int n = integer_ceil(remaining_matrices, size);
            remaining_matrices -= n;

            sendcount[i] = n * sizeof(Matrix);
            displs[i] = i == 0 ? 0 : i * sendcount[i - 1];

            local_mat_sizes[i] = n;
        }

        // Malloc arrays
        if ((global_results_array = malloc(num_targets * sizeof(int))) == nil)
        {
            perror("Malloc error global_results\n");
            exit(1);
        }
    }

    // BCast Kernel
    if (MPI_Bcast(&kernel, sizeof(kernel), MPI_BYTE, ROOT_RANK, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Failed broadcast kernel\n");
        exit(1);
    }
    // BCast SendCount
    if (MPI_Bcast(sendcount, size * sizeof(*sendcount), MPI_BYTE, ROOT_RANK, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Failed broadcast sendcount\n");
        exit(1);
    }

    // Scatter size of each local array
    int local_mat_size;
    if (MPI_Scatter(local_mat_sizes, sizeof(int), MPI_BYTE, &local_mat_size, sizeof(int), MPI_BYTE, ROOT_RANK, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Scatter error local_mat_sizes\n");
        exit(1);
    }

    // Malloc local matrices based on its own size
    Matrix *local_mat;
    if ((local_mat = malloc(local_mat_size * sizeof(Matrix))) == nil)
    {
        perror("Malloc error local_mat\n");
        exit(1);
    }

    // Scatterv input matrices based on the number of matrices in each local array of matrices
    if (MPI_Scatterv(arr_m, sendcount, displs, MPI_BYTE, local_mat, sizeof(*local_mat) * local_mat_size, MPI_BYTE, ROOT_RANK, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Scatter error arr_m\n");
        exit(1);
    }

    // Create local array to contain range result from OpenMP
    int *local_results_array;
    if ((local_results_array = malloc(sizeof(int) * local_mat_size)) == nil)
    {
        perror("Malloc error local_results\n");
        exit(1);
    }

    // Fill in the data based on OpenMP call for every matrices in the local array
    // <<<<<<<-------OPENMP HERE--------->>>>>>>
    for (int i = 0; i < local_mat_size; i++)
    {
        // local_results_array[i] = openmp(&local_mat[i]);
        local_results_array[i] = openmp(rank);
    }

    // Sort the local results, from 0 to total of array - 1
    merge_sort(local_results_array, 0, local_mat_size - 1);

    // Recalculate displacement and sendcount for Gatherv
    if (rank == ROOT_RANK)
    {
        for (int i = 0; i < size; i++)
        {
            displs[i] = displs[i] / sizeof(Matrix) * sizeof(int);
            sendcount[i] = sendcount[i] / sizeof(Matrix) * sizeof(int);
        }
    }

    // Gather merge-sorted result of each nodes
    if (MPI_Gatherv(local_results_array, local_mat_size * sizeof(int), MPI_BYTE, global_results_array, sendcount, displs, MPI_BYTE, ROOT_RANK, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Gather Error global_results\n");
        exit(1);
    }

    // Output the results
    if (rank == ROOT_RANK)
    {
        merge_sort(global_results_array, 0, num_targets - 1);
        int min_val = global_results_array[0];
        int max_val = global_results_array[num_targets - 1];
        int median = get_median(global_results_array, num_targets);
        long mean = get_floored_mean(global_results_array, num_targets);
        printf("%d\n%d\n%d\n%ld\n", min_val, max_val, median, mean);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Calculate time
    end_time = MPI_Wtime();
    duration = end_time - start_time;

    // Find maximum runtime from each process as the final runtime
    MPI_Reduce(&duration, &runtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == ROOT_RANK)
    {
        printf("Total runtime: %f\n", runtime);
    }

    MPI_Finalize();
}
