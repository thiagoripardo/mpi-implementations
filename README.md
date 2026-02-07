# mpi-implementations

Matrix operations implemented with MPI in C. The programs apply an iterative
stencil update over 2D or 3D matrices, splitting rows across MPI ranks and
exchanging boundary values each time step.

## Inputs and Parameters

Each program prompts for:

- `rows` (I): number of matrix rows.
- `cols` (J): number of matrix columns.
- `depth` (W): number of layers (3D programs only).
- `steps` (T): number of time steps for the stencil update loop.

The number of MPI processes (`size`) must evenly divide the row count:
`rows % size == 0`.

## Build and Run

```sh
make
make run-bi NP=2
```

Common commands:

```sh
# Build one program
make mpi_mtrx_tri

# Run with a custom process count
make run-tri NP=4

# Use mpiexec instead of mpirun
make run-bi MPI_RUN=mpiexec NP=4

# Remove build artifacts
make clean
```

All binaries are created under `bin/`.

In the run examples above, `NP=2` or `NP=4` controls the number of MPI
processes (ranks 0 through `size-1`).

Example inputs for `-n 2`:

- rows: 4
- cols: 4
- steps: 1

## Notes

- The update is an in-place stencil (Gauss-Seidel style), using boundary
  exchanges between neighboring ranks each step.
- See the Docs section for the stencil formula details and MPI rationale.
- MPI terms: `rank` is the ID of a process in `MPI_COMM_WORLD` (0 to `size-1`),
  and `size` is the total number of MPI processes launched. For example,
  `make run-bi NP=4` starts four ranks, numbered 0, 1, 2, and 3.
  Rank 0 reads input and distributes it; the other ranks receive parameters and
  update their own row blocks.

## Docs

- `@docs/MPI_RATIONALE.md`: why MPI is used in this repository.
- `@docs/STENCIL_GUIDE.md`: stencil formula details and Gauss-Seidel rationale.
- `@docs/mpi_mtrx_bi.md`: documentation for `mpi_mtrx_bi.c`.
- `@docs/mpi_mtrx_bi_v3.md`: documentation for `mpi_mtrx_bi_v3.c`.
- `@docs/mpi_mtrx_tri.md`: documentation for `mpi_mtrx_tri.c`.
- `@docs/mpi_mtrx_tri_embedded.md`: documentation for `mpi_mtrx_tri_embedded.c`.
