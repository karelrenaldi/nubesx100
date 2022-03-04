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
    int *displs, *sendcount, remaining_matrices = num_targets;
    size_t *local_mat_sizes = malloc(size * sizeof(size_t));
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

        for (int i = 0; i < size; i++)
        {
            int n = ceil(remaining_matrices / size);
            remaining_matrices -= n;
            sendcount[i] = n;

            displs[i] = 0;
            printf("%d\n", n);
            local_mat_sizes[i] = n * sizeof(Matrix);
        }
    }

    if (MPI_Bcast(&num_targets, 1, MPI_INT, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Failed broadcast\n");
        exit(1);
    }

    size_t local_mat_size;
    if (MPI_Scatter(local_mat_sizes, 1, MPI_UNSIGNED_LONG, &local_mat_size, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Scatter error 1\n");
        exit(1);
    }

    Matrix *local_mat;
    if ((local_mat = malloc(local_mat_size)) == nil)
    {
        perror("Malloc error 2\n");
        exit(1);
    }

    if (MPI_Scatterv(arr_m, sendcount, displs, MPI_BYTE, local_mat, sizeof(*local_mat), MPI_BYTE, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
    {
        perror("Scatter error 2\n");
        exit(1);
    }

    int num_m_per_proc = 2;
    for (int i = 0; i < num_m_per_proc; i++)
    {
        local_mat[i].col_eff = 3;
        local_mat[i].row_eff = 3;
    }

    printf("rank %d got numtarget of %d nummat %d\n", rank, num_targets, num_m_per_proc);
    if (rank == 1)
    {
        printf("from 1 %d %d\n", local_mat->col_eff, local_mat->row_eff);
        for (int i = 0; i < num_m_per_proc; i++)
        {
            print_matrix(&local_mat[i]);
        }
    }
    else if (rank == 0)
    {
        printf("from 0 %d %d\n", local_mat->col_eff, local_mat->row_eff);
        for (int i = 0; i < num_m_per_proc; i++)
        {
            print_matrix(&local_mat[i]);
        }
    }
    else
    {
        printf("from 3 %d %d\n", local_mat->col_eff, local_mat->row_eff);
        for (int i = 0; i < num_m_per_proc; i++)
        {
            print_matrix(&local_mat[i]);
        }
    }

    MPI_Finalize();
}