#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define nil NULL
#define NMAX 100
#define DATAMAX 1000
#define DATAMIN -1000

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

int randomv(int rank)
{
    return rank + rand();
}

int main(int argc, char **argv)
{
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        perror("Error initializing MPI\n");
        exit(1);
    }

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    Matrix *arr_m;
    Matrix kernel;
    int num_targets;
    int *displs, *sendcount, *sorted_range;

    int *local_mat_sizes = malloc(size * sizeof(int));
    sendcount = (int *)malloc(size * (sizeof(int)));
    displs = (int *)malloc(size * (sizeof(int)));

    if (rank == 0)
    {
        // Scan users' input
        int kernel_row, kernel_col, target_row, target_col;
        scanf("%d %d", &kernel_row, &kernel_col);
        kernel = input_matrix(kernel_row, kernel_col);

        scanf("%d %d %d", &num_targets, &target_row, &target_col);
        if ((arr_m = malloc(num_targets * sizeof(Matrix))) == nil)
        {
            printf("Malloc error 1\n");
            exit(1);
        }

        if ((sorted_range = malloc(size * sizeof(int))) == nil)
        {
            printf("Malloc error 2\n");
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
    }

    // BCast Kernel
    if (MPI_Bcast(&kernel, sizeof(kernel), MPI_BYTE, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Failed broadcast\n");
        exit(1);
    }

    // Scatter size of each local array
    int local_mat_size;
    if (MPI_Scatter(local_mat_sizes, sizeof(int), MPI_BYTE, &local_mat_size, sizeof(int), MPI_BYTE, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Scatter error 1\n");
        exit(1);
    }

    // Malloc local matrices based on its own size
    Matrix *local_mat;
    if ((local_mat = malloc(local_mat_size * sizeof(Matrix))) == nil)
    {
        perror("Malloc error 3\n");
        exit(1);
    }

    // Scatterv input matrices based on the number of matrices in each local array of matrices
    if (MPI_Scatterv(arr_m, sendcount, displs, MPI_BYTE, local_mat, sizeof(*local_mat) * local_mat_size, MPI_BYTE, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Scatter error 2\n");
        exit(1);
    }

    int local_result = randomv(rank);
    int *local_sorted_array;
    if ((local_sorted_array = malloc(sizeof(int))) == nil)
    {
        perror("Malloc error 4\n");
        exit(1);
    }
    printf("From rank %d %d", rank, local_result);

    int divisor = 2;
    int even_size = size % 2 == 0;
    while (divisor <= size)
    {
        if (even_size)
        {
            if (rank % 2 == 0)
            {
                int current_length = sizeof(&local_sorted_array) / sizeof(&local_sorted_array[0]);
                if (current_length < 2)
                {
                    if ((local_sorted_array = realloc(local_sorted_array, 2 * current_length * sizeof(int))) == nil)
                    {
                        perror("Realloc error from rank %d\n", rank);
                        exit(1);
                    }
                    // Recv from rank + 1
                    MPI_Recv(&)
                }
                else
                {
                    // Recv from rank + 2
                    MPI_Recv(&)
                }
            }
        }
        else
        {
            if (rank % 2 == 0)
            {
            }
        }

        divisor *= 2;
    }

    if (rank == 0)
    {
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}