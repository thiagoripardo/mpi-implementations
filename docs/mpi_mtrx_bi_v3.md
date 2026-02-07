# mpi_mtrx_bi_v3.c

## Description

Variant of `mpi_mtrx_bi.c` that adds a helper macro for index computation.
The stencil logic and MPI communication pattern are identical to the base 2D
version. This file exists primarily as a scaffolding experiment; the actual
runtime behavior is the same.

## Differences from mpi_mtrx_bi.c

- Index macro:
  `mpi_mtrx_bi.c` has no macro, while `mpi_mtrx_bi_v3.c` defines
  `pos(i, j, elementsPerRank)` as `((i * elementsPerRank) + j)`.
- Stencil logic: identical in both files.
- Memory layout: `float **` (pointer-per-row) in both files.

The `pos` macro is defined but not used in the stencil loops (the code still
uses `value[i][j]` directly). It was likely intended for a flattened-array
version.

## Memory layout

Same as `mpi_mtrx_bi.c`: an array of `float *` pointers with one row per
pointer.

## Input parameters

- `rows` (Rows): total number of matrix rows.
- `cols` (Columns): total number of matrix columns.
- `steps` (Steps): number of time steps.

## Constraints

- Requires **at least 2** MPI processes.
- `rows % size == 0`.

## Initialization

```text
value[i][j] = elementsPerRank * rank + counter
```

## Stencil update

Identical to `mpi_mtrx_bi.c`. See that file's documentation for the full
formula breakdown.

## Output

```text
Result for rank <rank> at position i:<i> j:<j> was <value>
```

## Build and Run (Makefile - Recommended)

```sh
make mpi_mtrx_bi_v3
make run-bi-v3 NP=<num_processes>
```

To use `mpiexec` instead of `mpirun`:

```sh
make run-bi-v3 MPI_RUN=mpiexec NP=<num_processes>
```

Binaries are generated under `bin/` (for this program: `bin/mpi_mtrx_bi_v3`).
Remember the process-count constraint still applies: `rows % size == 0`.

## Build and Run (Direct MPI Commands)

```sh
mpicc -O2 -Wall -o mpi_mtrx_bi_v3 mpi_mtrx_bi_v3.c
mpirun -n <num_processes> ./mpi_mtrx_bi_v3
```

Makefile equivalent:

```sh
make mpi_mtrx_bi_v3
make run-bi-v3 NP=<num_processes>
```
