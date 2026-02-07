MPICC ?= mpicc
CFLAGS ?= -O2 -Wall
MPI_RUN ?= mpirun
NP ?= 2
BINDIR ?= bin

PROGRAMS = mpi_mtrx_bi mpi_mtrx_bi_v3 mpi_mtrx_tri mpi_mtrx_tri_embedded
BINARIES = $(addprefix $(BINDIR)/,$(PROGRAMS))

.DEFAULT_GOAL := all

.PHONY: all clean help run-bi run-bi-v3 run-tri run-tri-embedded

all: $(BINARIES)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/mpi_mtrx_bi: mpi_mtrx_bi.c | $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ $<

$(BINDIR)/mpi_mtrx_bi_v3: mpi_mtrx_bi_v3.c | $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ $<

$(BINDIR)/mpi_mtrx_tri: mpi_mtrx_tri.c | $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ $<

$(BINDIR)/mpi_mtrx_tri_embedded: mpi_mtrx_tri_embedded.c | $(BINDIR)
	$(MPICC) $(CFLAGS) -o $@ $<

mpi_mtrx_bi: $(BINDIR)/mpi_mtrx_bi
mpi_mtrx_bi_v3: $(BINDIR)/mpi_mtrx_bi_v3
mpi_mtrx_tri: $(BINDIR)/mpi_mtrx_tri
mpi_mtrx_tri_embedded: $(BINDIR)/mpi_mtrx_tri_embedded

run-bi: $(BINDIR)/mpi_mtrx_bi
	$(MPI_RUN) -n $(NP) ./$(BINDIR)/mpi_mtrx_bi

run-bi-v3: $(BINDIR)/mpi_mtrx_bi_v3
	$(MPI_RUN) -n $(NP) ./$(BINDIR)/mpi_mtrx_bi_v3

run-tri: $(BINDIR)/mpi_mtrx_tri
	$(MPI_RUN) -n $(NP) ./$(BINDIR)/mpi_mtrx_tri

run-tri-embedded: $(BINDIR)/mpi_mtrx_tri_embedded
	$(MPI_RUN) -n $(NP) ./$(BINDIR)/mpi_mtrx_tri_embedded

clean:
	rm -rf $(BINDIR)

help:
	@echo "Targets:"
	@echo "  make all                  Build all binaries into $(BINDIR)/"
	@echo "  make mpi_mtrx_bi          Build bin/mpi_mtrx_bi"
	@echo "  make mpi_mtrx_bi_v3       Build bin/mpi_mtrx_bi_v3"
	@echo "  make mpi_mtrx_tri         Build bin/mpi_mtrx_tri"
	@echo "  make mpi_mtrx_tri_embedded Build bin/mpi_mtrx_tri_embedded"
	@echo "  make run-bi               Run bin/mpi_mtrx_bi with MPI"
	@echo "  make run-bi-v3            Run bin/mpi_mtrx_bi_v3 with MPI"
	@echo "  make run-tri              Run bin/mpi_mtrx_tri with MPI"
	@echo "  make run-tri-embedded     Run bin/mpi_mtrx_tri_embedded with MPI"
	@echo "  make clean                Remove $(BINDIR)/"
	@echo ""
	@echo "Overrides:"
	@echo "  make run-bi NP=4"
	@echo "  make run-bi MPI_RUN=mpiexec NP=4"
	@echo "  make CFLAGS='-O3 -Wall'"
