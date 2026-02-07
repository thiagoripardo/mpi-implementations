# Stencil Update Guide

## Overview

This document explains the stencil mathematics and communication dependencies
used by the matrix-update implementations in this repository.

## Core update idea

A stencil step updates each cell from its current value and nearby cells. In
2D, the neighborhood includes left, right, up, and down when available. The
update is a weighted average of the center value and existing neighbors.

Example (interior 2D cell):

```text
new = (left + 2*center + right + up + down) / 6
```

Corner and edge cells use fewer terms and a smaller divisor.

## Explicit 2D formulas (as implemented)

Interior (i in 1..M-2, j in 1..J-2):

```text
new(i,j) = (left + 2*center + right + up + down) / 6
```

Top edge (i=0, j in 1..J-2):

```text
new(0,j) = (left + 2*center + right + down) / 5
```

Bottom edge (i=M-1, j in 1..J-2):

```text
new(M-1,j) = (left + 2*center + right + up) / 5
```

Left edge (j=0, i in 1..M-2):

```text
new(i,0) = (2*center + right + up + down) / 5
```

Right edge (j=J-1, i in 1..M-2):

```text
new(i,J-1) = (left + 2*center + up + down) / 5
```

Corners:

```text
new(0,0)       = (2*center + right + down) / 4
new(0,J-1)     = (left + 2*center + down) / 4
new(M-1,0)     = (2*center + right + up) / 4
new(M-1,J-1)   = (left + 2*center + up + halo_next) / 5
```

For 3D, the update similarly uses neighbors in the third dimension (depth),
again averaging the available neighbors.

## 3D extension

For interior cells, the update includes the six axis-aligned neighbors:

```text
new = (left + right + up + down + front + back + 2*center) / 8
```

Edges and corners drop missing neighbors and adjust the divisor accordingly.

## Iteration model (Gauss-Seidel style)

The implementations update values in place. After a cell is updated, nearby
cells in the same step may read the new value. This ordering behavior is
characteristic of Gauss-Seidel iterations. By contrast, a Jacobi iteration
keeps separate current/next states and swaps only after finishing a full step.

## Halo exchange concept

Each MPI rank owns a contiguous block of rows. Boundary rows depend on values
owned by neighboring ranks, so halo values are exchanged between neighbors:

- One direction provides data from the last local row.
- The opposite direction provides data from the first local row.

These exchanged values provide the neighbor data needed to update boundary
cells at the process edges.
