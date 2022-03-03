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

    for (int i = 0; i < m->row_eff; i++)
    {
        for (int j = 0; j < m->col_eff; j++)
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

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    // Get number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get process rank
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get matrices from user inputs
    int kernel_row, kernel_col, num_targets, target_row, target_col;

    // reads kernel's row and column and initalize kernel matrix from input
    scanf("%d %d", &kernel_row, &kernel_col);
    Matrix kernel = input_matrix(kernel_row, kernel_col);

    // reads number of target matrices and their dimensions.
    // initialize array of matrices and array of data ranges (int)
    scanf("%d %d %d", &num_targets, &target_row, &target_col);
    Matrix *arr_mat = (Matrix *)malloc(num_targets * sizeof(Matrix));

    for (int i = 0; i < num_targets; i++)
    {
        arr_mat[i] = input_matrix(target_row, target_col);
    }

    // Get number of matrices per process
    int num_mat_per_proc = ceil(num_targets / size);

    // Init local matrices and scatter the global matrices
    Matrix *local_mats = (Matrix *)malloc(num_mat_per_proc * sizeof(Matrix));
    MPI_Scatter(arr_mat, sizeof(local_mats), MPI_BYTE, &local_mats, sizeof(local_mats), MPI_BYTE, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        // Range arrays -> array of convolution results
        int arr_range[num_targets];
        printf("From master \n");

        for (int i = 0; i < 3; i++)
        {
            print_matrix(&arr_mat[i]);
        }
    }
    else
    {
        printf("From slave %d = \n", rank);
        print_matrix(&local_mats[0]);
    }

    MPI_Finalize();
}