// Copyright (C) 2010, Steffen Knollmann
// Released under the terms of the GNU General Public License version 3.


/*--- Includes ----------------------------------------------------------*/
#include "partConfig.h"
#include "partBunch_tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef WITH_MPI
#  include <mpi.h>
#endif
#ifdef XMEM_TRACK_MEM
#  include "../libutil/xmem.h"
#endif

/*--- Local defines -----------------------------------------------------*/
#define NAME "libpart"


/*--- Macros ------------------------------------------------------------*/
#define RUNTEST(a, hasFailed)  \
    if (!(local_runtest(a))) { \
		hasFailed = true;      \
	} else {                   \
		if (!hasFailed)        \
			hasFailed = false; \
	}


/*--- Prototypes of loceal functions ------------------------------------*/
static bool
local_runtest(bool (*f)(void));


/*--- M A I N -----------------------------------------------------------*/
int
main(int argc, char **argv)
{
	bool hasFailed = false;
	int  rank      = 0;
	int  size      = 1;

#ifdef WITH_MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif

	if (rank == 0) {
		printf("\nTesting %s on %i %s\n",
		       NAME, size, size > 1 ? "tasks" : "task");
	}

	if (rank == 0) {
		printf("\nRunning tests for partBunch:\n");
	}
	RUNTEST(&partBunch_new_test, hasFailed);
	RUNTEST(&partBunch_del_test, hasFailed);
	RUNTEST(&partBunch_allocMem_test, hasFailed);
	RUNTEST(&partBunch_freeMem_test, hasFailed);
	RUNTEST(&partBunch_resize_test, hasFailed);
	RUNTEST(&partBunch_isAllocated_test, hasFailed);
	RUNTEST(&partBunch_getNumParticles_test, hasFailed);
	RUNTEST(&partBunch_at_test, hasFailed);
#ifdef XMEM_TRACK_MEM
	if (rank == 0)
		xmem_info(stdout);
	global_max_allocated_bytes = 0;
#endif

#ifdef WITH_MPI
	MPI_Finalize();
#endif

	if (hasFailed) {
		if (rank == 0)
			fprintf(stderr, "\nSome tests failed!\n\n");
		return EXIT_FAILURE;
	}
	if (rank == 0)
		printf("\nAll tests passed successfully!\n\n");

	return EXIT_SUCCESS;
} /* main */

/*--- Implementations of local functions --------------------------------*/
static bool
local_runtest(bool (*f)(void))
{
	bool hasPassed = f();
	int  rank      = 0;
	int  failedGlobal;
#ifdef WITH_MPI
	int  failedLocal = hasPassed ? 0 : 1;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Allreduce(&failedLocal, &failedGlobal, 1, MPI_INT, MPI_MAX,
	              MPI_COMM_WORLD);
	if (failedGlobal != 0)
		hasPassed = false;
#endif

	if (!hasPassed) {
		if (rank == 0)
			printf("!! FAILED !!\n");
	} else {
		if (rank == 0)
			printf("passed\n");
	}

	return hasPassed;
}
