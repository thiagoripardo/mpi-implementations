# Repository Guidelines

## Project Structure & Module Organization
- Root directory contains standalone MPI C programs for matrix operations: `mpi_mtrx_bi.c`, `mpi_mtrx_bi_v3.c`, `mpi_mtrx_tri.c`, and `mpi_mtrx_tri_embedded.c`.
- There is no dedicated `src/` or `tests/` tree; each `.c` file is a self-contained entry point with its own `main`.
- Documentation is minimal and lives in `README.md`.

## Build, Test, and Development Commands
This repo has no build scripts. Use your MPI toolchain directly.
- Compile a program: `mpicc -O2 -Wall -o mpi_mtrx_bi mpi_mtrx_bi.c`
- Run with MPI processes: `mpirun -n 4 ./mpi_mtrx_bi`
- Repeat for other files by swapping the target/source names.

If your environment uses a different launcher (e.g., `mpiexec`), prefer the one that matches your MPI distribution.

## Coding Style & Naming Conventions
- Language: C with MPI (`#include "mpi.h"`). Keep changes C89/C99 compatible unless there is a strong reason otherwise.
- Indentation follows the existing files (tabs/spaces are mixed). Match the surrounding style in the file you touch.
- File naming pattern uses `mpi_mtrx_*.c`; new programs should follow the same prefix.
- Comments are currently in Portuguese; keep new comments consistent with the file you edit.

## Testing Guidelines
- No automated tests or fixtures exist.
- Validate changes by compiling and running the specific program with representative inputs.
- When reporting results, note the program name, process count, and input sizes.

## Commit & Pull Request Guidelines
- Use Conventional Commits (e.g., `feat: add mpi_mtrx_bi_v4` or `fix: correct boundary update`).
- PRs should explain intent, list the files touched, and include the exact build/run commands used to verify behavior.
- If behavior changes or inputs/outputs change, include a brief before/after note.

## Configuration & Environment Notes
- Requires an MPI implementation (OpenMPI, MPICH, or equivalent). Document which one you used if issues are MPI-specific.
