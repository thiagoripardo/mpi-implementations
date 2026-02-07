# mpi_mtrx_tri_embedded.c

## Description

3D matrix stencil update using MPI with a **contiguous (embedded) memory
layout**. This is a variant of `mpi_mtrx_tri.c` that stores the 3D matrix in
a single contiguous 1D array instead of using triple-pointer allocation.
Produces identical output to `mpi_mtrx_tri.c` for the same inputs.

## Memory layout

The matrix is allocated as a single `float *` array of size
`localRows * cols * depth`:

```c
float *value;
value = (float *)malloc(localRows * cols * depth * sizeof(float));
```

Element access uses the `IDX` macro for linearized indexing:

```c
#define IDX(i, j, w) (((i) * cols + (j)) * depth + (w))

value[IDX(i, j, w)]  // equivalent to value[i][j][w] in mpi_mtrx_tri.c
```

## Input parameters

- `rows` (Rows): total number of matrix rows (I).
- `cols` (Columns): total number of matrix columns (J).
- `depth` (Depth): third dimension size (W).
- `steps` (Steps): number of time steps (T).

## Constraints

- Requires **at least 2** MPI processes.
- `rows % size == 0`.

## Initialization

```text
value[IDX(i, j, w)] = elementsPerRank * rank + counter
```

where `elementsPerRank = (rows * cols * depth) / size` and `counter`
increments from 0.

## Stencil update

Same formulas as `mpi_mtrx_tri.c`. The 3D stencil uses up to 6 axis-aligned
neighbors plus the center cell:

- **Interior cell**: `(left + right + up + down + front + back + 2*center) / 8`
- **Face cell** (one boundary): drops one neighbor, divisor = 7
- **Edge cell** (two boundaries): drops two neighbors, divisor = 6
- **Corner cell** (three boundaries): drops three neighbors, divisor = 5

The code handles all 8 combinations of degenerate dimensions
(`localRows <= 1`, `cols <= 1`, `depth <= 1`) with separate branches for
rank 0, middle ranks, and the last rank.

### Boundary exchange

Each time step:

- Rank 0 sends `value[IDX(localRows-1, cols-1, depth-1)]` to rank 1, receives
  `valueNext`.
- Middle ranks send `value[IDX(0, 0, 0)]` to `rank-1` and
  `value[IDX(localRows-1, cols-1, depth-1)]` to `rank+1`, receive `valuePrev`
  and `valueNext`.
- Last rank sends `value[IDX(0, 0, 0)]` to `rank-1`, receives `valuePrev`.

## Output

```text
Result for rank <rank> at position i:<i> j:<j> w:<w> was <value>
```

## Build and Run (Makefile - Recommended)

```sh
make mpi_mtrx_tri_embedded
make run-tri-embedded NP=<num_processes>
```

To use `mpiexec` instead of `mpirun`:

```sh
make run-tri-embedded MPI_RUN=mpiexec NP=<num_processes>
```

Binaries are generated under `bin/` (for this program: `bin/mpi_mtrx_tri_embedded`).
Remember the process-count constraint still applies: `rows % size == 0`.

## Build and Run (Direct MPI Commands)

```sh
mpicc -O2 -Wall -o mpi_mtrx_tri_embedded mpi_mtrx_tri_embedded.c
mpirun -n <num_processes> ./mpi_mtrx_tri_embedded
```

Makefile equivalent:

```sh
make mpi_mtrx_tri_embedded
make run-tri-embedded NP=<num_processes>
```

### Example (Direct MPI)

```sh
mpirun -n 2 ./mpi_mtrx_tri_embedded
# rows: 4, cols: 4, depth: 2, steps: 1
```
