# Why MPI

## Overview

This document explains why distributed-memory parallelism is a good match for
the stencil workloads used in this repository.

## Core rationale

- **Distributed data ownership:** Matrix rows are partitioned across ranks,
  which spreads memory usage beyond a single process.
- **Concurrent local computation:** Each rank advances its local block in
  parallel with other ranks.
- **Neighbor communication:** Halo exchange preserves correctness at partition
  boundaries where stencil dependencies cross rank ownership.
- **Scalable decomposition model:** The row-partitioned approach extends with
  rank count when decomposition constraints are satisfied (for example,
  `rows % size == 0` in these implementations).

## Tradeoffs

- **Communication overhead:** Frequent boundary exchange adds latency and can
  limit speedup when local work per rank is small.
- **Synchronization cost:** Time-step coordination across ranks can expose load
  imbalance.
- **Complexity cost:** Message-passing logic adds implementation complexity
  compared with single-process code.

## Applicability characteristics

MPI-based decomposition is most effective when matrices are large, iteration
counts are high, and per-step compute work is substantial relative to boundary
communication volume.
