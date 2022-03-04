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
    int num_targets;
    int *displs, *sendcount;

    int *local_mat_sizes = malloc(size * sizeof(int));
    sendcount = (int *)malloc(size * (sizeof(int)));
    displs = (int *)malloc(size * (sizeof(int)));

    if (rank == 0)
    {
        int target_row, target_col;

        scanf("%d %d %d", &num_targets, &target_row, &target_col);

        if ((arr_m = malloc(num_targets * sizeof(Matrix))) == nil)
        {
            printf("Malloc error 1\n");
            exit(1);
        }

        for (int i = 0; i < num_targets; i++)
        {
            arr_m[i] = input_matrix(target_row, target_col);
        }

        int remaining_matrices = num_targets;
        for (int i = 0; i < size; i++)
        {
            int n = remaining_matrices / size + (remaining_matrices % size != 0);
            remaining_matrices -= n;

            sendcount[i] = n * sizeof(Matrix);
            if (i != 0)
            {
                displs[i] = i * sendcount[i - 1];
            }

            local_mat_sizes[i] = n;
        }
    }

    if (MPI_Bcast(&num_targets, 1, MPI_INT, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Failed broadcast\n");
        exit(1);
    }

    int local_mat_size;
    if (MPI_Scatter(local_mat_sizes, sizeof(int), MPI_BYTE, &local_mat_size, sizeof(int), MPI_BYTE, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Scatter error 1\n");
        exit(1);
    }

    Matrix *local_mat;
    if ((local_mat = malloc(local_mat_size * sizeof(Matrix))) == nil)
    {
        perror("Malloc error 2\n");
        exit(1);
    }

    if (MPI_Scatterv(arr_m, sendcount, displs, MPI_BYTE, local_mat, sizeof(*local_mat) * local_mat_size, MPI_BYTE, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Scatter error 2\n");
        exit(1);
    }

    MPI_Finalize();
}