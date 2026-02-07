# mpi_mtrx_bi.c

## Description

2D matrix stencil update using MPI with dynamic memory allocation. Each cell
of a 2D matrix is iteratively updated using a weighted average of itself and
its neighbors (Gauss-Seidel style). The matrix rows are split evenly across
MPI ranks, and boundary values are exchanged between neighboring ranks at each
time step.

## Memory layout

The matrix is allocated as an array of `float *` pointers, one per local row.
Each row is a separate `malloc`'d `float` array of length `cols`. Access
pattern: `value[i][j]`.

## Input parameters

Prompted interactively at runtime (rank 0 reads, then broadcasts via
`MPI_Send`):

- `rows` (Rows): total number of matrix rows (I).
- `cols` (Columns): total number of matrix columns (J).
- `steps` (Steps): number of time steps (T).

## Constraints

- Requires **at least 2** MPI processes (`size >= 2`).
- `rows` must be evenly divisible by the number of processes (`rows % size == 0`).

## Initialization

Each element is set to a sequential integer based on rank offset:

```text
value[i][j] = elementsPerRank * rank + counter
```

where `elementsPerRank = (rows * cols) / size` and `counter` increments from 0.

## Stencil update

At each time step, every rank updates its local block in place. The update
formula depends on the cell's position:

- **Interior cell**: `(left + 2*center + right + up + down) / 6`
- **Edge cell**: drops the missing neighbor, divisor decreases by 1
- **Corner cell**: drops two missing neighbors, divisor decreases by 2

Boundary exchange: rank 0 sends its bottom-right value to rank 1 and receives
the next value back. Middle ranks exchange with both neighbors. The last rank
exchanges only with its predecessor.

The code handles degenerate cases (single row per rank, single column) with
separate branches.

## Output

Each rank prints its local results:

```text
Result for rank <rank> at position i:<i> j:<j> was <value>
```

## Build and Run (Makefile - Recommended)

```sh
make mpi_mtrx_bi
make run-bi NP=<num_processes>
```

To use `mpiexec` instead of `mpirun`:

```sh
make run-bi MPI_RUN=mpiexec NP=<num_processes>
```

Binaries are generated under `bin/` (for this program: `bin/mpi_mtrx_bi`).
Remember the process-count constraint still applies: `rows % size == 0`.

## Build and Run (Direct MPI Commands)

```sh
mpicc -O2 -Wall -o mpi_mtrx_bi mpi_mtrx_bi.c
mpirun -n <num_processes> ./mpi_mtrx_bi
```

Makefile equivalent:

```sh
make mpi_mtrx_bi
make run-bi NP=<num_processes>
```

### Example (Direct MPI)

```sh
mpirun -n 2 ./mpi_mtrx_bi
# rows: 4, cols: 4, steps: 1
```
