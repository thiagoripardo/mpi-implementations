#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

/* This code uses a dynamically allocated matrix, where rows and columns are
entered by the user and depth is the third dimension. Depending on the number
of processes, it divides the matrix rows evenly among them. The number of
processes (size) must be a multiple of the row count and column count. */

int main(int argc, char **argv) {

  int rank, size, i, j, w, rows, cols, depth, elementsPerRank, localRows, step,
      steps, counter = 0;
  float ***value, valueNext, valuePrev;

  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    printf("Enter the desired number of matrix rows:\n");
    scanf("%d", &rows);
    printf("Enter the desired number of matrix columns:\n");
    scanf("%d", &cols);
    printf("Enter the desired matrix depth:\n");
    scanf("%d", &depth);
    for (i = 1; i < size; i++) {
      MPI_Send(&rows, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      MPI_Send(&cols, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      MPI_Send(&depth, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
    }
  } else {
    MPI_Recv(&rows, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&cols, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&depth, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
  }

  if (size == 1) {
    if (rank == 0)
      printf("This program requires at least 2 MPI processes.\n");
    MPI_Finalize();
    return 0;
  }

  if (rows % size == 0) {

    localRows = rows / size;
    elementsPerRank = (rows * cols * depth) / size;

    value = (float ***)malloc((localRows) * sizeof(float **));
    for (i = 0; i < localRows; i++) {
      value[i] = (float **)malloc((cols) * sizeof(float *));
      for (j = 0; j < cols; j++) {
        value[i][j] = (float *)malloc((depth) * sizeof(float));
      }
    }

    if (rank == 0) {
      printf("Enter the time steps:\n");
      scanf("%d", &steps);
      for (i = 1; i < size; i++)
        MPI_Send(&steps, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
    }

    else
      MPI_Recv(&steps, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

    for (i = 0; i < localRows; i++) {
      for (j = 0; j < cols; j++) {
        for (w = 0; w < depth; w++) {
          value[i][j][w] = (elementsPerRank * rank + counter);
          counter++;
        }
      }
    }

    for (step = 1; step <= steps; step++) {

      if (rank == 0) {

        MPI_Send(&value[localRows - 1][cols - 1][depth - 1], 1, MPI_FLOAT,
                 (rank + 1), 1, MPI_COMM_WORLD);
        MPI_Recv(&valueNext, 1, MPI_FLOAT, (rank + 1), 1, MPI_COMM_WORLD,
                 &status);

        if ((localRows > 1) && (cols > 1) && (depth > 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[0][1][0] +
                             value[1][0][0] + value[0][0][1]) /
                            5);
          // edge along depth at i=0, j=0 (w varies)
          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] =
                ((value[0][0][w - 1] + 2 * value[0][0][w] + value[0][0][w + 1] +
                  value[0][1][w] + value[1][0][w]) /
                 6);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[0][1][depth - 1] + value[1][0][depth - 1]) /
               5);
          // edge along columns at i=0, w=0 (j varies)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] =
                ((value[0][j - 1][0] + 2 * value[0][j][0] + value[0][j + 1][0] +
                  value[1][j][0] + value[0][j][1]) /
                 6);
          }
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0] +
                value[0][cols - 1][1] + value[1][cols - 1][0]) /
               5);
          // edge along columns at i=0, w=depth-1 (j varies)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][depth - 1] =
                ((value[0][j - 1][depth - 1] + 2 * value[0][j][depth - 1] +
                  value[0][j + 1][depth - 1] + value[1][j][depth - 1] +
                  value[0][j][depth - 1]) /
                 6);
          }
          value[0][cols - 1][depth - 1] =
              ((value[0][cols - 1][depth - 2] +
                2 * value[0][cols - 1][depth - 1] +
                value[0][cols - 2][depth - 1] + value[1][cols - 1][depth - 1]) /
               5);
          // edge along depth at i=0, j=cols-1 (w varies)
          for (w = 1; w < depth - 1; w++) {
            value[0][cols - 1][w] =
                ((value[0][cols - 1][w - 1] + 2 * value[0][cols - 1][w] +
                  value[0][cols - 1][w + 1] + value[0][cols - 2][w] +
                  value[1][cols - 1][w]) /
                 6);
          }
          // interior volume (i in 1..localRows-2, j in 1..cols-2, w
          // in 1..depth-2)
          for (j = 1; j < cols - 1; j++) {
            for (w = 1; w < depth - 1; w++) {
              value[0][j][w] = ((value[0][j][w - 1] + 2 * value[0][j][w] +
                                 value[0][j][w + 1] + value[0][j + 1][w] +
                                 value[0][j - 1][w] + value[1][j][w]) /
                                7);
            }
          }

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i][1][0] +
                               value[i + 1][0][0] + value[i][0][1]) /
                              5);
            // left face (j=0)
            for (w = 1; w < depth - 1; w++) {
              value[i][0][w] =
                  ((value[i][0][w - 1] + 2 * value[i][0][w] +
                    value[i][0][w + 1] + value[i][1][w] + value[i + 1][0][w]) /
                   6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i][1][depth - 1] + value[i + 1][0][depth - 1]) /
                 5);
            // front face (w=0)
            for (j = 1; j < cols - 1; j++) {
              value[i][j][0] =
                  ((value[i][j - 1][0] + 2 * value[i][j][0] +
                    value[i][j + 1][0] + value[i + 1][j][0] + value[i][j][1]) /
                   6);
            }
            value[i][cols - 1][0] =
                ((value[i][cols - 2][0] + 2 * value[i][cols - 1][0] +
                  value[i][cols - 1][1] + value[i + 1][cols - 1][0]) /
                 5);
            // back face (w=depth-1)
            for (j = 1; j < cols - 1; j++) {
              value[i][j][depth - 1] =
                  ((value[i][j - 1][depth - 1] + 2 * value[i][j][depth - 1] +
                    value[i][j + 1][depth - 1] + value[i + 1][j][depth - 1] +
                    value[i][j][depth - 1]) /
                   6);
            }
            value[i][cols - 1][depth - 1] =
                ((value[i][cols - 1][depth - 2] +
                  2 * value[i][cols - 1][depth - 1] +
                  value[i][cols - 2][depth - 1] +
                  value[i + 1][cols - 1][depth - 1]) /
                 5);
            // right face (j=cols-1)
            for (w = 1; w < depth - 1; w++) {
              value[i][cols - 1][w] =
                  ((value[i][cols - 1][w - 1] + 2 * value[i][cols - 1][w] +
                    value[i][cols - 1][w + 1] + value[i][cols - 2][w] +
                    value[i + 1][cols - 1][w]) /
                   6);
            }
            // interior volume (i in 1..localRows-2, j in 1..cols-2, w
            // in 1..depth-2)
            for (j = 1; j < cols - 1; j++) {
              for (w = 1; w < depth - 1; w++) {
                value[i][j][w] = ((value[i][j][w - 1] + 2 * value[i][j][w] +
                                   value[i][j][w + 1] + value[i][j + 1][w] +
                                   value[i][j - 1][w] + value[i - 1][j][w] +
                                   value[i + 1][j][w]) /
                                  8);
              }
            }
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 1][1][0] +
                value[localRows - 2][0][0] + value[localRows - 1][0][1]) /
               5);
          // bottom-left edge (i=localRows-1, j=0)
          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][0][w] =
                ((value[localRows - 1][0][w - 1] +
                  2 * value[localRows - 1][0][w] +
                  value[localRows - 1][0][w + 1] + value[localRows - 1][1][w] +
                  value[localRows - 2][0][w]) /
                 6);
          }
          value[localRows - 1][0][depth - 1] =
              ((value[localRows - 1][0][depth - 2] +
                2 * value[localRows - 1][0][depth - 1] +
                value[localRows - 1][1][depth - 1] +
                value[localRows - 2][0][depth - 1]) /
               5);
          // bottom-front edge (i=localRows-1, w=0)
          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][j][0] =
                ((value[localRows - 1][j - 1][0] +
                  2 * value[localRows - 1][j][0] +
                  value[localRows - 1][j + 1][0] + value[localRows - 2][j][0] +
                  value[localRows - 1][j][1]) /
                 6);
          }
          value[localRows - 1][cols - 1][0] =
              ((value[localRows - 1][cols - 2][0] +
                2 * value[localRows - 1][cols - 1][0] +
                value[localRows - 1][cols - 1][1] +
                value[localRows - 2][cols - 1][0]) /
               5);
          // bottom-back edge (i=localRows-1, w=depth-1)
          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][j][depth - 1] =
                ((value[localRows - 1][j - 1][depth - 1] +
                  2 * value[localRows - 1][j][depth - 1] +
                  value[localRows - 1][j + 1][depth - 1] +
                  value[localRows - 2][j][depth - 1] +
                  value[localRows - 1][j][depth - 1]) /
                 6);
          }
          // bottom-right edge (i=localRows-1, j=cols-1)
          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][cols - 1][w] =
                ((value[localRows - 1][cols - 1][w - 1] +
                  2 * value[localRows - 1][cols - 1][w] +
                  value[localRows - 1][cols - 1][w + 1] +
                  value[localRows - 1][cols - 2][w] +
                  value[localRows - 2][cols - 1][w]) /
                 6);
          }
          // interior volume (i in 1..localRows-2, j in 1..cols-2, w
          // in 1..depth-2)
          for (j = 1; j < cols - 1; j++) {
            for (w = 1; w < depth - 1; w++) {
              value[localRows - 1][j][w] = ((value[localRows - 1][j][w - 1] +
                                             2 * value[localRows - 1][j][w] +
                                             value[localRows - 1][j][w + 1] +
                                             value[localRows - 1][j + 1][w] +
                                             value[localRows - 1][j - 1][w] +
                                             value[localRows - 2][j][w]) /
                                            7);
            }
          }

          value[localRows - 1][cols - 1][depth - 1] =
              ((value[localRows - 1][cols - 1][depth - 2] +
                2 * value[localRows - 1][cols - 1][depth - 1] +
                value[localRows - 1][cols - 2][depth - 1] +
                value[localRows - 2][cols - 1][depth - 1] + valueNext) /
               6);
        }

        if ((localRows > 1) && (cols <= 1) && (depth > 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[1][0][0] + value[0][0][1]) / 4);

          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1] + value[1][0][w]) /
                              5);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[1][0][depth - 1]) /
               4);

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i + 1][0][0] +
                               value[i][0][1] + value[i - 1][0][0]) /
                              5);

            for (w = 1; w < depth - 1; w++) {
              value[i][0][w] = ((value[i][0][w - 1] + 2 * value[i][0][w] +
                                 value[i][0][w + 1] + value[i + 1][0][w] +
                                 value[i - 1][0][w]) /
                                6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i + 1][0][depth - 1] + value[i - 1][0][depth - 1]) /
                 5);
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 2][0][0] +
                value[localRows - 1][0][1]) /
               4);

          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][0][w] =
                ((value[localRows - 1][0][w - 1] +
                  2 * value[localRows - 1][0][w] +
                  value[localRows - 1][0][w + 1] + value[localRows - 2][0][w]) /
                 5);
          }
          value[localRows - 1][0][depth - 1] =
              ((value[localRows - 1][0][depth - 2] +
                2 * value[localRows - 1][0][depth - 1] +
                value[localRows - 2][0][depth - 1] + valueNext) /
               5);
        }

        if ((localRows <= 1) && (cols > 1) && (depth > 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[0][1][0] + value[0][0][1]) / 4);

          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1] + value[0][1][w]) /
                              5);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[0][1][depth - 1]) /
               4);

          // interior columns (j in 1..cols-2)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] = ((2 * value[0][j][0] + value[0][j - 1][0] +
                               value[0][j + 1][0] + value[0][j][1]) /
                              5);

            for (w = 1; w < depth - 1; w++) {
              value[0][j][w] = ((value[0][j][w - 1] + 2 * value[0][j][w] +
                                 value[0][j][w + 1] + value[0][j + 1][w] +
                                 value[0][j - 1][w]) /
                                6);
            }
            value[0][j][depth - 1] =
                ((value[0][j][depth - 2] + 2 * value[0][j][depth - 1] +
                  value[0][j + 1][depth - 1] + value[0][j - 1][depth - 1]) /
                 5);
          }

          // right face (j=cols-1)
          value[0][cols - 1][0] =
              ((2 * value[0][cols - 1][0] + value[0][cols - 2][0] +
                value[0][cols - 1][1]) /
               4);

          for (w = 1; w < depth - 1; w++) {
            value[0][cols - 1][w] =
                ((value[0][cols - 1][w - 1] + 2 * value[0][cols - 1][w] +
                  value[0][cols - 1][w + 1] + value[0][cols - 2][w]) /
                 5);
          }
          value[0][cols - 1][depth - 1] =
              ((value[0][cols - 1][depth - 2] +
                2 * value[0][cols - 1][depth - 1] +
                value[0][cols - 2][depth - 1] + valueNext) /
               5);
        }

        if ((localRows > 1) && (cols > 1) && (depth <= 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[1][0][0] + value[0][1][0]) / 4);

          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] = ((value[0][j - 1][0] + 2 * value[0][j][0] +
                               value[0][j + 1][0] + value[1][j][0]) /
                              5);
          }
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0] +
                value[1][cols - 1][0]) /
               4);

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i + 1][0][0] +
                               value[i][1][0] + value[i - 1][0][0]) /
                              5);

            for (j = 1; j < cols - 1; j++) {
              value[i][j][0] = ((value[i][j - 1][0] + 2 * value[i][j][0] +
                                 value[i][j + 1][0] + value[i + 1][j][0] +
                                 value[i - 1][j][0]) /
                                6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i + 1][0][depth - 1] + value[i - 1][0][depth - 1]) /
                 5);
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 2][0][0] +
                value[localRows - 1][1][0]) /
               4);

          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][0][j] =
                ((value[localRows - 1][j - 1][0] +
                  2 * value[localRows - 1][j][0] +
                  value[localRows - 1][j + 1][0] + value[localRows - 2][j][0]) /
                 5);
          }
          value[localRows - 1][cols - 1][0] =
              ((value[localRows - 1][cols - 2][0] +
                2 * value[localRows - 1][cols - 1][0] +
                value[localRows - 2][cols - 1][0] + valueNext) /
               5);
        }

        if ((localRows > 1) && (cols <= 1) && (depth <= 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[1][0][0]) / 3);
          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++)
            value[i][0][0] = ((value[i - 1][0][0] + 2 * value[i][0][0] +
                               value[i + 1][0][0]) /
                              4);
          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((value[localRows - 2][0][0] + 2 * value[localRows - 1][0][0] +
                valueNext) /
               4);
        }

        if ((localRows <= 1) && (cols > 1) && (depth <= 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[0][1][0]) / 3);
          // interior columns (j in 1..cols-2)
          for (j = 1; j < cols - 1; j++)
            value[0][j][0] = ((value[0][j - 1][0] + 2 * value[0][j][0] +
                               value[0][j + 1][0]) /
                              4);
          // right face (j=cols-1)
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0] + valueNext) /
               4);
        }

        if ((localRows <= 1) && (cols <= 1) && (depth > 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[0][0][1]) / 3);
          // interior depth (w in 1..depth-2)
          for (w = 1; w < depth - 1; w++)
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1]) /
                              4);
          // back face (w=depth-1)
          value[0][0][depth - 1] = ((value[0][0][depth - 2] +
                                     2 * value[0][0][depth - 1] + valueNext) /
                                    4);
        }

        if ((localRows <= 1) && (cols <= 1) && (depth <= 1))
          value[0][0][0] = ((2 * value[0][0][0] + valueNext) / 3);
      }

      if ((rank > 0) && (rank < (size - 1))) {

        MPI_Send(&value[0][0][0], 1, MPI_FLOAT, (rank - 1), 1, MPI_COMM_WORLD);
        MPI_Send(&value[localRows - 1][cols - 1][depth - 1], 1, MPI_FLOAT,
                 (rank + 1), 1, MPI_COMM_WORLD);
        MPI_Recv(&valuePrev, 1, MPI_FLOAT, (rank - 1), 1, MPI_COMM_WORLD,
                 &status);
        MPI_Recv(&valueNext, 1, MPI_FLOAT, (rank + 1), 1, MPI_COMM_WORLD,
                 &status);

        if ((localRows > 1) && (cols > 1) && (depth > 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[0][1][0] +
                             value[1][0][0] + value[0][0][1] + valuePrev) /
                            6);
          // edge along depth at i=0, j=0 (w varies)
          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] =
                ((value[0][0][w - 1] + 2 * value[0][0][w] + value[0][0][w + 1] +
                  value[0][1][w] + value[1][0][w]) /
                 6);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[0][1][depth - 1] + value[1][0][depth - 1]) /
               5);
          // edge along columns at i=0, w=0 (j varies)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] =
                ((value[0][j - 1][0] + 2 * value[0][j][0] + value[0][j + 1][0] +
                  value[1][j][0] + value[0][j][1]) /
                 6);
          }
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0] +
                value[0][cols - 1][1] + value[1][cols - 1][0]) /
               5);
          // edge along columns at i=0, w=depth-1 (j varies)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][depth - 1] =
                ((value[0][j - 1][depth - 1] + 2 * value[0][j][depth - 1] +
                  value[0][j + 1][depth - 1] + value[1][j][depth - 1] +
                  value[0][j][depth - 1]) /
                 6);
          }
          value[0][cols - 1][depth - 1] =
              ((value[0][cols - 1][depth - 2] +
                2 * value[0][cols - 1][depth - 1] +
                value[0][cols - 2][depth - 1] + value[1][cols - 1][depth - 1]) /
               5);
          // edge along depth at i=0, j=cols-1 (w varies)
          for (w = 1; w < depth - 1; w++) {
            value[0][cols - 1][w] =
                ((value[0][cols - 1][w - 1] + 2 * value[0][cols - 1][w] +
                  value[0][cols - 1][w + 1] + value[0][cols - 2][w] +
                  value[1][cols - 1][w]) /
                 6);
          }
          // interior volume (i in 1..localRows-2, j in 1..cols-2, w
          // in 1..depth-2)
          for (j = 1; j < cols - 1; j++) {
            for (w = 1; w < depth - 1; w++) {
              value[0][j][w] = ((value[0][j][w - 1] + 2 * value[0][j][w] +
                                 value[0][j][w + 1] + value[0][j + 1][w] +
                                 value[0][j - 1][w] + value[1][j][w]) /
                                7);
            }
          }

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i][1][0] +
                               value[i + 1][0][0] + value[i][0][1]) /
                              5);
            // left face (j=0)
            for (w = 1; w < depth - 1; w++) {
              value[i][0][w] =
                  ((value[i][0][w - 1] + 2 * value[i][0][w] +
                    value[i][0][w + 1] + value[i][1][w] + value[i + 1][0][w]) /
                   6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i][1][depth - 1] + value[i + 1][0][depth - 1]) /
                 5);
            // front face (w=0)
            for (j = 1; j < cols - 1; j++) {
              value[i][j][0] =
                  ((value[i][j - 1][0] + 2 * value[i][j][0] +
                    value[i][j + 1][0] + value[i + 1][j][0] + value[i][j][1]) /
                   6);
            }
            value[i][cols - 1][0] =
                ((value[i][cols - 2][0] + 2 * value[i][cols - 1][0] +
                  value[i][cols - 1][1] + value[i + 1][cols - 1][0]) /
                 5);
            // back face (w=depth-1)
            for (j = 1; j < cols - 1; j++) {
              value[i][j][depth - 1] =
                  ((value[i][j - 1][depth - 1] + 2 * value[i][j][depth - 1] +
                    value[i][j + 1][depth - 1] + value[i + 1][j][depth - 1] +
                    value[i][j][depth - 1]) /
                   6);
            }
            value[i][cols - 1][depth - 1] =
                ((value[i][cols - 1][depth - 2] +
                  2 * value[i][cols - 1][depth - 1] +
                  value[i][cols - 2][depth - 1] +
                  value[i + 1][cols - 1][depth - 1]) /
                 5);
            // right face (j=cols-1)
            for (w = 1; w < depth - 1; w++) {
              value[i][cols - 1][w] =
                  ((value[i][cols - 1][w - 1] + 2 * value[i][cols - 1][w] +
                    value[i][cols - 1][w + 1] + value[i][cols - 2][w] +
                    value[i + 1][cols - 1][w]) /
                   6);
            }
            // interior volume (i in 1..localRows-2, j in 1..cols-2, w
            // in 1..depth-2)
            for (j = 1; j < cols - 1; j++) {
              for (w = 1; w < depth - 1; w++) {
                value[i][j][w] = ((value[i][j][w - 1] + 2 * value[i][j][w] +
                                   value[i][j][w + 1] + value[i][j + 1][w] +
                                   value[i][j - 1][w] + value[i - 1][j][w] +
                                   value[i + 1][j][w]) /
                                  8);
              }
            }
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 1][1][0] +
                value[localRows - 2][0][0] + value[localRows - 1][0][1]) /
               5);
          // bottom-left edge (i=localRows-1, j=0)
          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][0][w] =
                ((value[localRows - 1][0][w - 1] +
                  2 * value[localRows - 1][0][w] +
                  value[localRows - 1][0][w + 1] + value[localRows - 1][1][w] +
                  value[localRows - 2][0][w]) /
                 6);
          }
          value[localRows - 1][0][depth - 1] =
              ((value[localRows - 1][0][depth - 2] +
                2 * value[localRows - 1][0][depth - 1] +
                value[localRows - 1][1][depth - 1] +
                value[localRows - 2][0][depth - 1]) /
               5);
          // bottom-front edge (i=localRows-1, w=0)
          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][j][0] =
                ((value[localRows - 1][j - 1][0] +
                  2 * value[localRows - 1][j][0] +
                  value[localRows - 1][j + 1][0] + value[localRows - 2][j][0] +
                  value[localRows - 1][j][1]) /
                 6);
          }
          value[localRows - 1][cols - 1][0] =
              ((value[localRows - 1][cols - 2][0] +
                2 * value[localRows - 1][cols - 1][0] +
                value[localRows - 1][cols - 1][1] +
                value[localRows - 2][cols - 1][0]) /
               5);
          // bottom-back edge (i=localRows-1, w=depth-1)
          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][j][depth - 1] =
                ((value[localRows - 1][j - 1][depth - 1] +
                  2 * value[localRows - 1][j][depth - 1] +
                  value[localRows - 1][j + 1][depth - 1] +
                  value[localRows - 2][j][depth - 1] +
                  value[localRows - 1][j][depth - 1]) /
                 6);
          }
          // bottom-right edge (i=localRows-1, j=cols-1)
          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][cols - 1][w] =
                ((value[localRows - 1][cols - 1][w - 1] +
                  2 * value[localRows - 1][cols - 1][w] +
                  value[localRows - 1][cols - 1][w + 1] +
                  value[localRows - 1][cols - 2][w] +
                  value[localRows - 2][cols - 1][w]) /
                 6);
          }
          // interior volume (i in 1..localRows-2, j in 1..cols-2, w
          // in 1..depth-2)
          for (j = 1; j < cols - 1; j++) {
            for (w = 1; w < depth - 1; w++) {
              value[localRows - 1][j][w] = ((value[localRows - 1][j][w - 1] +
                                             2 * value[localRows - 1][j][w] +
                                             value[localRows - 1][j][w + 1] +
                                             value[localRows - 1][j + 1][w] +
                                             value[localRows - 1][j - 1][w] +
                                             value[localRows - 2][j][w]) /
                                            7);
            }
          }

          value[localRows - 1][cols - 1][depth - 1] =
              ((value[localRows - 1][cols - 1][depth - 2] +
                2 * value[localRows - 1][cols - 1][depth - 1] +
                value[localRows - 1][cols - 2][depth - 1] +
                value[localRows - 2][cols - 1][depth - 1] + valueNext) /
               6);
        }

        if ((localRows > 1) && (cols <= 1) && (depth > 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[1][0][0] +
                             value[0][0][1] + valuePrev) /
                            5);

          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1] + value[1][0][w]) /
                              5);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[1][0][depth - 1]) /
               4);

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i + 1][0][0] +
                               value[i][0][1] + value[i - 1][0][0]) /
                              5);

            for (w = 1; w < depth - 1; w++) {
              value[i][0][w] = ((value[i][0][w - 1] + 2 * value[i][0][w] +
                                 value[i][0][w + 1] + value[i + 1][0][w] +
                                 value[i - 1][0][w]) /
                                6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i + 1][0][depth - 1] + value[i - 1][0][depth - 1]) /
                 5);
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 2][0][0] +
                value[localRows - 1][0][1]) /
               4);

          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][0][w] =
                ((value[localRows - 1][0][w - 1] +
                  2 * value[localRows - 1][0][w] +
                  value[localRows - 1][0][w + 1] + value[localRows - 2][0][w]) /
                 5);
          }
          value[localRows - 1][0][depth - 1] =
              ((value[localRows - 1][0][depth - 2] +
                2 * value[localRows - 1][0][depth - 1] +
                value[localRows - 2][0][depth - 1] + valueNext) /
               5);
        }

        if ((localRows <= 1) && (cols > 1) && (depth > 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[0][1][0] +
                             value[0][0][1] + valuePrev) /
                            5);

          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1] + value[0][1][w]) /
                              5);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[0][1][depth - 1]) /
               4);

          // interior columns (j in 1..cols-2)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] = ((2 * value[0][j][0] + value[0][j - 1][0] +
                               value[0][j + 1][0] + value[0][j][1]) /
                              5);

            for (w = 1; w < depth - 1; w++) {
              value[0][j][w] = ((value[0][j][w - 1] + 2 * value[0][j][w] +
                                 value[0][j][w + 1] + value[0][j + 1][w] +
                                 value[0][j - 1][w]) /
                                6);
            }
            value[0][j][depth - 1] =
                ((value[0][j][depth - 2] + 2 * value[0][j][depth - 1] +
                  value[0][j + 1][depth - 1] + value[0][j - 1][depth - 1]) /
                 5);
          }

          // right face (j=cols-1)
          value[0][cols - 1][0] =
              ((2 * value[0][cols - 1][0] + value[0][cols - 2][0] +
                value[0][cols - 1][1]) /
               4);

          for (w = 1; w < depth - 1; w++) {
            value[0][cols - 1][w] =
                ((value[0][cols - 1][w - 1] + 2 * value[0][cols - 1][w] +
                  value[0][cols - 1][w + 1] + value[0][cols - 2][w]) /
                 5);
          }
          value[0][cols - 1][depth - 1] =
              ((value[0][cols - 1][depth - 2] +
                2 * value[0][cols - 1][depth - 1] +
                value[0][cols - 2][depth - 1] + valueNext) /
               5);
        }

        if ((localRows > 1) && (cols > 1) && (depth <= 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[1][0][0] +
                             value[0][1][0] + valuePrev) /
                            5);

          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] = ((value[0][j - 1][0] + 2 * value[0][j][0] +
                               value[0][j + 1][0] + value[1][j][0]) /
                              5);
          }
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0] +
                value[1][cols - 1][0]) /
               4);

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i + 1][0][0] +
                               value[i][1][0] + value[i - 1][0][0]) /
                              5);

            for (j = 1; j < cols - 1; j++) {
              value[i][j][0] = ((value[i][j - 1][0] + 2 * value[i][j][0] +
                                 value[i][j + 1][0] + value[i + 1][j][0] +
                                 value[i - 1][j][0]) /
                                6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i + 1][0][depth - 1] + value[i - 1][0][depth - 1]) /
                 5);
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 2][0][0] +
                value[localRows - 1][1][0]) /
               4);

          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][0][j] =
                ((value[localRows - 1][j - 1][0] +
                  2 * value[localRows - 1][j][0] +
                  value[localRows - 1][j + 1][0] + value[localRows - 2][j][0]) /
                 5);
          }
          value[localRows - 1][cols - 1][0] =
              ((value[localRows - 1][cols - 2][0] +
                2 * value[localRows - 1][cols - 1][0] +
                value[localRows - 2][cols - 1][0] + valueNext) /
               5);
        }

        if ((localRows > 1) && (cols <= 1) && (depth <= 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[1][0][0] + valuePrev) / 4);
          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++)
            value[i][0][0] = ((value[i - 1][0][0] + 2 * value[i][0][0] +
                               value[i + 1][0][0]) /
                              4);
          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((value[localRows - 2][0][0] + 2 * value[localRows - 1][0][0] +
                valueNext) /
               4);
        }

        if ((localRows <= 1) && (cols > 1) && (depth <= 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[0][1][0] + valuePrev) / 4);
          // interior columns (j in 1..cols-2)
          for (j = 1; j < cols - 1; j++)
            value[0][j][0] = ((value[0][j - 1][0] + 2 * value[0][j][0] +
                               value[0][j + 1][0]) /
                              4);
          // right face (j=cols-1)
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0] + valueNext) /
               4);
        }

        if ((localRows <= 1) && (cols <= 1) && (depth > 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[0][0][1] + valuePrev) / 4);
          // interior depth (w in 1..depth-2)
          for (w = 1; w < depth - 1; w++)
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1]) /
                              4);
          // back face (w=depth-1)
          value[0][0][depth - 1] = ((value[0][0][depth - 2] +
                                     2 * value[0][0][depth - 1] + valueNext) /
                                    4);
        }

        if ((localRows <= 1) && (cols <= 1) && (depth <= 1))
          value[0][0][0] = ((2 * value[0][0][0] + valuePrev + valueNext) / 4);
      }

      if (rank == (size - 1)) {
        MPI_Send(&value[0][0][0], 1, MPI_FLOAT, (rank - 1), 1, MPI_COMM_WORLD);
        MPI_Recv(&valuePrev, 1, MPI_FLOAT, (rank - 1), 1, MPI_COMM_WORLD,
                 &status);

        if ((localRows > 1) && (cols > 1) && (depth > 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[0][1][0] +
                             value[1][0][0] + value[0][0][1] + valuePrev) /
                            6);
          // edge along depth at i=0, j=0 (w varies)
          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] =
                ((value[0][0][w - 1] + 2 * value[0][0][w] + value[0][0][w + 1] +
                  value[0][1][w] + value[1][0][w]) /
                 6);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[0][1][depth - 1] + value[1][0][depth - 1]) /
               5);
          // edge along columns at i=0, w=0 (j varies)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] =
                ((value[0][j - 1][0] + 2 * value[0][j][0] + value[0][j + 1][0] +
                  value[1][j][0] + value[0][j][1]) /
                 6);
          }
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0] +
                value[0][cols - 1][1] + value[1][cols - 1][0]) /
               5);
          // edge along columns at i=0, w=depth-1 (j varies)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][depth - 1] =
                ((value[0][j - 1][depth - 1] + 2 * value[0][j][depth - 1] +
                  value[0][j + 1][depth - 1] + value[1][j][depth - 1] +
                  value[0][j][depth - 1]) /
                 6);
          }
          value[0][cols - 1][depth - 1] =
              ((value[0][cols - 1][depth - 2] +
                2 * value[0][cols - 1][depth - 1] +
                value[0][cols - 2][depth - 1] + value[1][cols - 1][depth - 1]) /
               5);
          // edge along depth at i=0, j=cols-1 (w varies)
          for (w = 1; w < depth - 1; w++) {
            value[0][cols - 1][w] =
                ((value[0][cols - 1][w - 1] + 2 * value[0][cols - 1][w] +
                  value[0][cols - 1][w + 1] + value[0][cols - 2][w] +
                  value[1][cols - 1][w]) /
                 6);
          }
          // interior volume (i in 1..localRows-2, j in 1..cols-2, w
          // in 1..depth-2)
          for (j = 1; j < cols - 1; j++) {
            for (w = 1; w < depth - 1; w++) {
              value[0][j][w] = ((value[0][j][w - 1] + 2 * value[0][j][w] +
                                 value[0][j][w + 1] + value[0][j + 1][w] +
                                 value[0][j - 1][w] + value[1][j][w]) /
                                7);
            }
          }

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i][1][0] +
                               value[i + 1][0][0] + value[i][0][1]) /
                              5);
            // left face (j=0)
            for (w = 1; w < depth - 1; w++) {
              value[i][0][w] =
                  ((value[i][0][w - 1] + 2 * value[i][0][w] +
                    value[i][0][w + 1] + value[i][1][w] + value[i + 1][0][w]) /
                   6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i][1][depth - 1] + value[i + 1][0][depth - 1]) /
                 5);
            // front face (w=0)
            for (j = 1; j < cols - 1; j++) {
              value[i][j][0] =
                  ((value[i][j - 1][0] + 2 * value[i][j][0] +
                    value[i][j + 1][0] + value[i + 1][j][0] + value[i][j][1]) /
                   6);
            }
            value[i][cols - 1][0] =
                ((value[i][cols - 2][0] + 2 * value[i][cols - 1][0] +
                  value[i][cols - 1][1] + value[i + 1][cols - 1][0]) /
                 5);
            // back face (w=depth-1)
            for (j = 1; j < cols - 1; j++) {
              value[i][j][depth - 1] =
                  ((value[i][j - 1][depth - 1] + 2 * value[i][j][depth - 1] +
                    value[i][j + 1][depth - 1] + value[i + 1][j][depth - 1] +
                    value[i][j][depth - 1]) /
                   6);
            }
            value[i][cols - 1][depth - 1] =
                ((value[i][cols - 1][depth - 2] +
                  2 * value[i][cols - 1][depth - 1] +
                  value[i][cols - 2][depth - 1] +
                  value[i + 1][cols - 1][depth - 1]) /
                 5);
            // right face (j=cols-1)
            for (w = 1; w < depth - 1; w++) {
              value[i][cols - 1][w] =
                  ((value[i][cols - 1][w - 1] + 2 * value[i][cols - 1][w] +
                    value[i][cols - 1][w + 1] + value[i][cols - 2][w] +
                    value[i + 1][cols - 1][w]) /
                   6);
            }
            // interior volume (i in 1..localRows-2, j in 1..cols-2, w
            // in 1..depth-2)
            for (j = 1; j < cols - 1; j++) {
              for (w = 1; w < depth - 1; w++) {
                value[i][j][w] = ((value[i][j][w - 1] + 2 * value[i][j][w] +
                                   value[i][j][w + 1] + value[i][j + 1][w] +
                                   value[i][j - 1][w] + value[i - 1][j][w] +
                                   value[i + 1][j][w]) /
                                  8);
              }
            }
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 1][1][0] +
                value[localRows - 2][0][0] + value[localRows - 1][0][1]) /
               5);
          // bottom-left edge (i=localRows-1, j=0)
          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][0][w] =
                ((value[localRows - 1][0][w - 1] +
                  2 * value[localRows - 1][0][w] +
                  value[localRows - 1][0][w + 1] + value[localRows - 1][1][w] +
                  value[localRows - 2][0][w]) /
                 6);
          }
          value[localRows - 1][0][depth - 1] =
              ((value[localRows - 1][0][depth - 2] +
                2 * value[localRows - 1][0][depth - 1] +
                value[localRows - 1][1][depth - 1] +
                value[localRows - 2][0][depth - 1]) /
               5);
          // bottom-front edge (i=localRows-1, w=0)
          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][j][0] =
                ((value[localRows - 1][j - 1][0] +
                  2 * value[localRows - 1][j][0] +
                  value[localRows - 1][j + 1][0] + value[localRows - 2][j][0] +
                  value[localRows - 1][j][1]) /
                 6);
          }
          value[localRows - 1][cols - 1][0] =
              ((value[localRows - 1][cols - 2][0] +
                2 * value[localRows - 1][cols - 1][0] +
                value[localRows - 1][cols - 1][1] +
                value[localRows - 2][cols - 1][0]) /
               5);
          // bottom-back edge (i=localRows-1, w=depth-1)
          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][j][depth - 1] =
                ((value[localRows - 1][j - 1][depth - 1] +
                  2 * value[localRows - 1][j][depth - 1] +
                  value[localRows - 1][j + 1][depth - 1] +
                  value[localRows - 2][j][depth - 1] +
                  value[localRows - 1][j][depth - 1]) /
                 6);
          }
          // bottom-right edge (i=localRows-1, j=cols-1)
          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][cols - 1][w] =
                ((value[localRows - 1][cols - 1][w - 1] +
                  2 * value[localRows - 1][cols - 1][w] +
                  value[localRows - 1][cols - 1][w + 1] +
                  value[localRows - 1][cols - 2][w] +
                  value[localRows - 2][cols - 1][w]) /
                 6);
          }
          // interior volume (i in 1..localRows-2, j in 1..cols-2, w
          // in 1..depth-2)
          for (j = 1; j < cols - 1; j++) {
            for (w = 1; w < depth - 1; w++) {
              value[localRows - 1][j][w] = ((value[localRows - 1][j][w - 1] +
                                             2 * value[localRows - 1][j][w] +
                                             value[localRows - 1][j][w + 1] +
                                             value[localRows - 1][j + 1][w] +
                                             value[localRows - 1][j - 1][w] +
                                             value[localRows - 2][j][w]) /
                                            7);
            }
          }

          value[localRows - 1][cols - 1][depth - 1] =
              ((value[localRows - 1][cols - 1][depth - 2] +
                2 * value[localRows - 1][cols - 1][depth - 1] +
                value[localRows - 1][cols - 2][depth - 1] +
                value[localRows - 2][cols - 1][depth - 1]) /
               5);
        }

        if ((localRows > 1) && (cols <= 1) && (depth > 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[1][0][0] +
                             value[0][0][1] + valuePrev) /
                            5);

          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1] + value[1][0][w]) /
                              5);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[1][0][depth - 1]) /
               4);

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i + 1][0][0] +
                               value[i][0][1] + value[i - 1][0][0]) /
                              5);

            for (w = 1; w < depth - 1; w++) {
              value[i][0][w] = ((value[i][0][w - 1] + 2 * value[i][0][w] +
                                 value[i][0][w + 1] + value[i + 1][0][w] +
                                 value[i - 1][0][w]) /
                                6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i + 1][0][depth - 1] + value[i - 1][0][depth - 1]) /
                 5);
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 2][0][0] +
                value[localRows - 1][0][1]) /
               4);

          for (w = 1; w < depth - 1; w++) {
            value[localRows - 1][0][w] =
                ((value[localRows - 1][0][w - 1] +
                  2 * value[localRows - 1][0][w] +
                  value[localRows - 1][0][w + 1] + value[localRows - 2][0][w]) /
                 5);
          }
          value[localRows - 1][0][depth - 1] =
              ((value[localRows - 1][0][depth - 2] +
                2 * value[localRows - 1][0][depth - 1] +
                value[localRows - 2][0][depth - 1]) /
               4);
        }

        if ((localRows <= 1) && (cols > 1) && (depth > 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[0][1][0] +
                             value[0][0][1] + valuePrev) /
                            5);

          for (w = 1; w < depth - 1; w++) {
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1] + value[0][1][w]) /
                              5);
          }
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1] +
                value[0][1][depth - 1]) /
               4);

          // interior columns (j in 1..cols-2)
          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] = ((2 * value[0][j][0] + value[0][j - 1][0] +
                               value[0][j + 1][0] + value[0][j][1]) /
                              5);

            for (w = 1; w < depth - 1; w++) {
              value[0][j][w] = ((value[0][j][w - 1] + 2 * value[0][j][w] +
                                 value[0][j][w + 1] + value[0][j + 1][w] +
                                 value[0][j - 1][w]) /
                                6);
            }
            value[0][j][depth - 1] =
                ((value[0][j][depth - 2] + 2 * value[0][j][depth - 1] +
                  value[0][j + 1][depth - 1] + value[0][j - 1][depth - 1]) /
                 5);
          }

          // right face (j=cols-1)
          value[0][cols - 1][0] =
              ((2 * value[0][cols - 1][0] + value[0][cols - 2][0] +
                value[0][cols - 1][1]) /
               4);

          for (w = 1; w < depth - 1; w++) {
            value[0][cols - 1][w] =
                ((value[0][cols - 1][w - 1] + 2 * value[0][cols - 1][w] +
                  value[0][cols - 1][w + 1] + value[0][cols - 2][w]) /
                 5);
          }
          value[0][cols - 1][depth - 1] = ((value[0][cols - 1][depth - 2] +
                                            2 * value[0][cols - 1][depth - 1] +
                                            value[0][cols - 2][depth - 1]) /
                                           4);
        }

        if ((localRows > 1) && (cols > 1) && (depth <= 1)) {

          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] = ((2 * value[0][0][0] + value[1][0][0] +
                             value[0][1][0] + valuePrev) /
                            5);

          for (j = 1; j < cols - 1; j++) {
            value[0][j][0] = ((value[0][j - 1][0] + 2 * value[0][j][0] +
                               value[0][j + 1][0] + value[1][j][0]) /
                              5);
          }
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0] +
                value[1][cols - 1][0]) /
               4);

          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++) {
            value[i][0][0] = ((2 * value[i][0][0] + value[i + 1][0][0] +
                               value[i][1][0] + value[i - 1][0][0]) /
                              5);

            for (j = 1; j < cols - 1; j++) {
              value[i][j][0] = ((value[i][j - 1][0] + 2 * value[i][j][0] +
                                 value[i][j + 1][0] + value[i + 1][j][0] +
                                 value[i - 1][j][0]) /
                                6);
            }
            value[i][0][depth - 1] =
                ((value[i][0][depth - 2] + 2 * value[i][0][depth - 1] +
                  value[i + 1][0][depth - 1] + value[i - 1][0][depth - 1]) /
                 5);
          }

          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((2 * value[localRows - 1][0][0] + value[localRows - 2][0][0] +
                value[localRows - 1][1][0]) /
               4);

          for (j = 1; j < cols - 1; j++) {
            value[localRows - 1][0][j] =
                ((value[localRows - 1][j - 1][0] +
                  2 * value[localRows - 1][j][0] +
                  value[localRows - 1][j + 1][0] + value[localRows - 2][j][0]) /
                 5);
          }
          value[localRows - 1][cols - 1][0] =
              ((value[localRows - 1][cols - 2][0] +
                2 * value[localRows - 1][cols - 1][0] +
                value[localRows - 2][cols - 1][0]) /
               4);
        }

        if ((localRows > 1) && (cols <= 1) && (depth <= 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[1][0][0] + valuePrev) / 4);
          // interior rows (i in 1..localRows-2)
          for (i = 1; i < localRows - 1; i++)
            value[i][0][0] = ((value[i - 1][0][0] + 2 * value[i][0][0] +
                               value[i + 1][0][0]) /
                              4);
          // bottom face (i=localRows-1)
          value[localRows - 1][0][0] =
              ((value[localRows - 2][0][0] + 2 * value[localRows - 1][0][0]) /
               3);
        }

        if ((localRows <= 1) && (cols > 1) && (depth <= 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[0][1][0] + valuePrev) / 4);
          // interior columns (j in 1..cols-2)
          for (j = 1; j < cols - 1; j++)
            value[0][j][0] = ((value[0][j - 1][0] + 2 * value[0][j][0] +
                               value[0][j + 1][0]) /
                              4);
          // right face (j=cols-1)
          value[0][cols - 1][0] =
              ((value[0][cols - 2][0] + 2 * value[0][cols - 1][0]) / 3);
        }

        if ((localRows <= 1) && (cols <= 1) && (depth > 1)) {
          // corner at i=0, j=0, w=0 (top/front/left)
          value[0][0][0] =
              ((2 * value[0][0][0] + value[0][0][1] + valuePrev) / 4);
          // interior depth (w in 1..depth-2)
          for (w = 1; w < depth - 1; w++)
            value[0][0][w] = ((value[0][0][w - 1] + 2 * value[0][0][w] +
                               value[0][0][w + 1]) /
                              4);
          // back face (w=depth-1)
          value[0][0][depth - 1] =
              ((value[0][0][depth - 2] + 2 * value[0][0][depth - 1]) / 3);
        }

        if ((localRows <= 1) && (cols <= 1) && (depth <= 1))
          value[0][0][0] = ((2 * value[0][0][0] + valuePrev) / 3);
      }
    }

    for (i = 0; i < localRows; i++) {
      for (j = 0; j < cols; j++) {
        for (w = 0; w < depth; w++) {
          printf("Result for rank %d at position i:%d j:%d w:%d was %f\n", rank,
                 i, j, w, value[i][j][w]);
        }
      }
    }
    MPI_Finalize();
  }

  else {
    if (rank == 0)
      printf("The number of processes is not a multiple of the matrix size\n");
    MPI_Finalize();
  }
  return 0;
}
