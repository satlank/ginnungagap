// Copyright (C) 2010, Steffen Knollmann
// Released under the terms of the GNU General Public License version 3.



/*--- Includes ----------------------------------------------------------*/
#include "cosmo_config.h"
#include "cosmoPk_tests.h"
#include "cosmoModel_tests.h"
#include <stdio.h>
#include <stdlib.h>


/*--- Macros ------------------------------------------------------------*/
#define RUNTEST(a, hasFailed) \
	if (!(a())) { \
		printf("failed\n"); \
		hasFailed = true; \
	} else { \
		printf("passed\n"); \
		if (!hasFailed) \
			hasFailed = false; \
	}



/*--- M A I N -----------------------------------------------------------*/
int
main(void)
{
	bool hasFailed;

	printf("Running tests for cosmoPk:\n");
	RUNTEST(cosmoPk_newFromFile_test, hasFailed);
	RUNTEST(cosmoPk_newFromArrays_test, hasFailed);
	RUNTEST(cosmoPk_del_test, hasFailed);
	RUNTEST(cosmoPk_eval_test, hasFailed);
	RUNTEST(cosmoPk_calcMomentFiltered_test, hasFailed);

	printf("\nRunning tests for cosmoModel:\n");
	RUNTEST(cosmoModel_newFromFile_test, hasFailed);
	RUNTEST(cosmoModel_del_test, hasFailed);
	RUNTEST(cosmoModel_calcAgeFromExpansion_test, hasFailed);
	RUNTEST(cosmoModel_calcExpansionFromAge_test, hasFailed);
	RUNTEST(cosmoModel_calcADot_test, hasFailed);
	RUNTEST(cosmoModel_calcOmegas_test, hasFailed);
	RUNTEST(cosmoModel_calcGrowth_test, hasFailed);

	if (hasFailed) {
		fprintf(stderr, "\nSome tests failed!\n\n");
		return EXIT_FAILURE;
	}
	printf("\nAll tests passed successfully!\n\n");
	return EXIT_SUCCESS;
}