#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define pos(i, j, N) ((i * N) + j)

/* Neste codigo foi utilizado uma matriz com alocação dinâmica, no qual I será o
numero de linhas(entrado pelo usuário) e J o numero de colunas(entrado pelo
usuario). Dependendo da escolha do numero de processos ele dividirá as linhas da
matriz igualmente dentro desses processos. Sendo o numero de processos(size) um
multiplo de I e J. */

int main(int argc, char **argv) {

  int rank, size, i, j, I, J, P, N, M, t, T, cont = 0;
  float **value, v_next, v_previous;

  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  P = size;

  if (rank == 0) {
    printf("Insira o tamanho desejado para as linhas da Matriz:\n");
    scanf("%d", &I);
    printf("Insira o tamanho desejado para as colunas da Matriz:\n");
    scanf("%d", &J);
    for (i = 1; i < size; i++) {
      MPI_Send(&I, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      MPI_Send(&J, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
    }
  }

  else {
    MPI_Recv(&I, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&J, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
  }

  if (I % P == 0) {

    M = I / P;
    N = (I * J) / P;

    value = (float **)malloc((M) * sizeof(float *));

    for (i = 0; i < M; i++)
      value[i] = (float *)malloc((J) * sizeof(float));

    if (rank == 0) {
      printf("Insira os passos de tempo:\n");
      scanf("%d", &T);
      for (i = 1; i < size; i++)
        MPI_Send(&T, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
    }

    else
      MPI_Recv(&T, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

    for (i = 0; i < M; i++) {
      for (j = 0; j < J; j++) {
        value[i][j] = (N * rank + cont);
        cont++;
      }
    }

    for (t = 1; t <= T; t++) {

      if (rank == 0) {

        MPI_Send(&value[M - 1][J - 1], 1, MPI_FLOAT, (rank + 1), 1,
                 MPI_COMM_WORLD);
        MPI_Recv(&v_next, 1, MPI_FLOAT, (rank + 1), 1, MPI_COMM_WORLD, &status);

        if ((M > 1) && (J > 1)) {
          // 0
          value[0][0] = ((2 * value[0][0] + value[0][1] + value[1][0]) / 4);
          for (j = 1; j < J - 1; j++)
            value[0][j] = ((value[0][j - 1] + 2 * value[0][j] +
                            value[0][j + 1] + value[1][j]) /
                           5);
          value[0][J - 1] =
              ((value[0][J - 2] + 2 * value[0][J - 1] + value[1][J - 1]) / 4);
          // entre 0 e M-1
          for (i = 1; i < M - 1; i++) {
            value[i][0] = ((2 * value[i][0] + value[i][1] + value[i + 1][0] +
                            value[i - 1][0]) /
                           5);
            for (j = 1; j < J - 1; j++)
              value[i][j] =
                  ((value[i][j - 1] + 2 * value[i][j] + value[i][j + 1] +
                    value[i + 1][j] + value[i - 1][j]) /
                   6);
            value[i][J - 1] = ((value[i][J - 2] + 2 * value[i][J - 1] +
                                value[i + 1][J - 1] + value[i - 1][J - 1]) /
                               5);
          }
          // M-1
          value[M - 1][0] =
              ((2 * value[M - 1][0] + value[M - 1][1] + value[M - 2][0]) / 4);
          for (j = 1; j < J - 1; j++)
            value[M - 1][j] = ((value[M - 1][j - 1] + 2 * value[M - 1][j] +
                                value[M - 1][j + 1] + value[M - 2][j]) /
                               5);
          value[M - 1][J - 1] =
              ((value[M - 1][J - 2] + 2 * value[M - 1][J - 1] +
                value[M - 2][J - 1] + v_next) /
               5);
        }

        if ((M > 1) && (J <= 1)) {
          // 0
          value[0][0] = ((2 * value[0][0] + value[1][0]) / 3);
          // entre 0 e M-1
          for (i = 1; i < M - 1; i++)
            value[i][0] =
                ((value[i - 1][0] + 2 * value[i][0] + value[i + 1][0]) / 4);
          // M-1
          value[M - 1][0] =
              ((value[M - 2][0] + 2 * value[M - 1][0] + v_next) / 4);
        }

        if ((M <= 1) && (J > 1)) {
          // 0
          value[0][0] = ((2 * value[0][0] + value[0][1]) / 3);
          // entre 0 e J-1
          for (j = 1; j < J - 1; j++)
            value[0][j] =
                ((value[0][j - 1] + 2 * value[0][j] + value[0][j + 1]) / 4);
          // J-1
          value[0][J - 1] =
              ((value[0][J - 2] + 2 * value[0][J - 1] + v_next) / 4);
        }

        if ((M <= 1) && (J <= 1)) {
          value[0][0] = ((2 * value[0][0] + value[0][1] + v_next) / 4);
        }
      }

      if ((rank > 0) && (rank < (size - 1))) {
        MPI_Send(&value[0][0], 1, MPI_FLOAT, (rank - 1), 1, MPI_COMM_WORLD);
        MPI_Send(&value[M - 1][J - 1], 1, MPI_FLOAT, (rank + 1), 1,
                 MPI_COMM_WORLD);
        MPI_Recv(&v_previous, 1, MPI_FLOAT, (rank - 1), 1, MPI_COMM_WORLD,
                 &status);
        MPI_Recv(&v_next, 1, MPI_FLOAT, (rank + 1), 1, MPI_COMM_WORLD, &status);

        if (M > 1) {
          // 0
          value[0][0] =
              ((2 * value[0][0] + value[0][1] + value[1][0] + v_previous) / 5);
          for (j = 1; j < J - 1; j++)
            value[0][j] = ((value[0][j - 1] + 2 * value[0][j] +
                            value[0][j + 1] + value[1][j]) /
                           5);
          value[0][J - 1] =
              ((value[0][J - 2] + 2 * value[0][J - 1] + value[1][J - 1]) / 4);
          // entre 0 e M-1
          for (i = 1; i < M - 1; i++) {
            value[i][0] = ((2 * value[i][0] + value[i][1] + value[i + 1][0] +
                            value[i - 1][0]) /
                           5);
            for (j = 1; j < J - 1; j++)
              value[i][j] =
                  ((value[i][j - 1] + 2 * value[i][j] + value[i][j + 1] +
                    value[i + 1][j] + value[i - 1][j]) /
                   6);
            value[i][J - 1] = ((value[i][J - 2] + 2 * value[i][J - 1] +
                                value[i + 1][J - 1] + value[i - 1][J - 1]) /
                               5);
          }
          // M-1
          value[M - 1][0] =
              ((2 * value[M - 1][0] + value[M - 1][1] + value[M - 2][0]) / 4);
          for (j = 1; j < J - 1; j++)
            value[M - 1][j] = ((value[M - 1][j - 1] + 2 * value[M - 1][j] +
                                value[M - 1][j + 1] + value[M - 2][j]) /
                               5);
          value[M - 1][J - 1] =
              ((value[M - 1][J - 2] + 2 * value[M - 1][J - 1] +
                value[M - 2][J - 1] + v_next) /
               5);
        }

        if ((M > 1) && (J <= 1)) {
          // 0
          value[0][0] = ((2 * value[0][0] + value[1][0] + v_previous) / 4);
          // entre 0 e M-1
          for (i = 1; i < M - 1; i++)
            value[i][0] =
                ((value[i - 1][0] + 2 * value[i][0] + value[i + 1][0]) / 4);
          // M-1
          value[M - 1][0] =
              ((value[M - 2][0] + 2 * value[M - 1][0] + v_next) / 4);
        }

        if ((M <= 1) && (J > 1)) {
          // 0
          value[0][0] = ((2 * value[0][0] + value[0][1] + v_previous) / 4);
          // entre 0 e J-1
          for (j = 1; j < J - 1; j++)
            value[0][j] =
                ((value[0][j - 1] + 2 * value[0][j] + value[0][j + 1]) / 4);
          // J-1
          value[0][J - 1] =
              ((value[0][J - 2] + 2 * value[0][J - 1] + v_next) / 4);
        }

        if ((M <= 1) && (J <= 1)) {
          value[0][0] =
              ((2 * value[0][0] + value[0][1] + v_previous + v_next) / 5);
        }
      }

      if (rank == (size - 1)) {
        MPI_Send(&value[0][0], 1, MPI_FLOAT, (rank - 1), 1, MPI_COMM_WORLD);
        MPI_Recv(&v_previous, 1, MPI_FLOAT, (rank - 1), 1, MPI_COMM_WORLD,
                 &status);

        if (M > 1) {
          // 0
          value[0][0] =
              ((2 * value[0][0] + value[0][1] + value[1][0] + v_previous) / 5);
          for (j = 1; j < J - 1; j++)
            value[0][j] = ((value[0][j - 1] + 2 * value[0][j] +
                            value[0][j + 1] + value[1][j]) /
                           5);
          value[0][J - 1] =
              ((value[0][J - 2] + 2 * value[0][J - 1] + value[1][J - 1]) / 4);
          // entre 0 e M-1
          for (i = 1; i < M - 1; i++) {
            value[i][0] = ((2 * value[i][0] + value[i][1] + value[i + 1][0] +
                            value[i - 1][0]) /
                           5);
            for (j = 1; j < J - 1; j++)
              value[i][j] =
                  ((value[i][j - 1] + 2 * value[i][j] + value[i][j + 1] +
                    value[i + 1][j] + value[i - 1][j]) /
                   6);
            value[i][J - 1] = ((value[i][J - 2] + 2 * value[i][J - 1] +
                                value[i + 1][J - 1] + value[i - 1][J - 1]) /
                               5);
          }
          // M-1
          value[M - 1][0] =
              ((2 * value[M - 1][0] + value[M - 1][1] + value[M - 2][0]) / 4);
          for (j = 1; j < J - 1; j++)
            value[M - 1][j] = ((value[M - 1][j - 1] + 2 * value[M - 1][j] +
                                value[M - 1][j + 1] + value[M - 2][j]) /
                               5);
          value[M - 1][J - 1] =
              ((value[M - 1][J - 2] + 2 * value[M - 1][J - 1] +
                value[M - 2][J - 1]) /
               4);
        }

        if ((M > 1) && (J <= 1)) {
          // 0
          value[0][0] = ((2 * value[0][0] + value[1][0] + v_previous) / 4);
          // entre 0 e M-1
          for (i = 1; i < M - 1; i++)
            value[i][0] =
                ((value[i - 1][0] + 2 * value[i][0] + value[i + 1][0]) / 4);
          // M-1
          value[M - 1][0] = ((value[M - 2][0] + 2 * value[M - 1][0]) / 3);
        }

        if ((M <= 1) && (J > 1)) {
          // 0
          value[0][0] = ((2 * value[0][0] + value[0][1] + v_previous) / 4);
          // entre 0 e J-1
          for (j = 1; j < J - 1; j++)
            value[0][j] =
                ((value[0][j - 1] + 2 * value[0][j] + value[0][j + 1]) / 4);
          // J-1
          value[0][J - 1] = ((value[0][J - 2] + 2 * value[0][J - 1]) / 3);
        }

        if ((M <= 1) && (J <= 1)) {
          value[0][0] = ((2 * value[0][0] + value[0][1] + v_previous) / 4);
        }
      }
    }

    for (i = 0; i < M; i++) {
      for (j = 0; j < J; j++)
        printf("O resultado para o rank %d e posicao i:%d j: %d foi %f\n", rank,
               i, j, value[i][j]);
    }
    MPI_Finalize();
  }

  else {
    if (rank == 0)
      printf("Os numero de processos nao sao multiplos do tamanho da Matriz\n");
    MPI_Finalize();
  }
  return 0;
}
