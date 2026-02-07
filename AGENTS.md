# Repository Guidelines

## Project Structure & Module Organization

- Root directory contains standalone MPI C programs for matrix operations:
  `mpi_mtrx_bi.c`, `mpi_mtrx_bi_v3.c`, `mpi_mtrx_tri.c`, and
  `mpi_mtrx_tri_embedded.c`.
- There is no dedicated `src/` or `tests/` tree; each `.c` file is a
  self-contained entry point with its own `main`.
- `README.md` provides the project overview and common usage.
- `docs/` contains conceptual docs (`MPI_RATIONALE.md`,
  `STENCIL_GUIDE.md`) and per-program references (`mpi_mtrx_*.md`).

## Build, Test, and Development Commands

Use the repository `Makefile` as the primary workflow.

- Build all programs: `make`
- Build one program: `make <binary>` (e.g., `make mpi_mtrx_tri`)
- Run a program: `make run-<target> NP=<nprocs>` (e.g., `make run-tri NP=4`)
- Override launcher: `make run-bi MPI_RUN=mpiexec NP=4`
- Clean artifacts: `make clean`
- Build outputs are generated in `bin/` and must not be committed.

Direct MPI commands are still valid as a fallback:
`mpicc -O2 -Wall -o <binary> <source>.c` and
`mpirun -n <nprocs> ./<binary>`.

## Coding Style & Naming Conventions

- Language: C with MPI (`#include "mpi.h"`). Keep changes C89/C99 compatible
  unless there is a strong reason otherwise.
- Indentation follows the existing files (tabs/spaces are mixed). Match the
  surrounding style in the file you touch.
- File naming pattern: `mpi_mtrx_*.c`. New programs must follow this prefix.

## Testing Guidelines

- No automated tests or fixtures exist.
- Validate changes by building and running the specific program with
  representative inputs.
- Prefer `make` and the `run-*` targets for validation runs.
- When reporting results, note the program name, process count, input sizes,
  and launcher used if non-default.

## Commit & Pull Request Guidelines

- Use Conventional Commits (e.g., `feat: add mpi_mtrx_bi_v4` or
  `fix: correct boundary update`).
- If behavior or I/O changes, include a brief before/after note.

## Configuration & Environment Notes

- Requires an MPI implementation (OpenMPI, MPICH, or equivalent). Document
  which one you used if issues are MPI-specific.
- Common `Makefile` overrides: `MPICC`, `CFLAGS`, `MPI_RUN`, `NP`, `BINDIR`.
