// Copyright (C) 2013, Steffen Knollmann
// Released under the terms of the GNU General Public License version 3.
// This file is part of `ginnungagap'.


/*--- Doxygen file description ------------------------------------------*/

/**
 * @file generateICs/generateICsCore.c
 * @ingroup  toolsGICSCore
 * @brief  Implements core functions for generateICs.
 */


/*--- Includes ----------------------------------------------------------*/
#include "generateICsConfig.h"
#include "generateICsCore.h"
#include <assert.h>
#include "../../src/libutil/xmem.h"
#include "../../src/libutil/lIdx.h"


/*--- Local defines -----------------------------------------------------*/


/*--- Prototypes of local functions -------------------------------------*/


/*--- Implementations of exported functions -----------------------------*/
extern void
generateICsCore_toParticles(generateICsCore_const_t d)
{
	generateICsCore_initPosID(d);
	generateICsCore_vel2pos(d);
	generateICsCore_convertVel(d);
}

extern void
generateICsCore_initPosID(generateICsCore_const_t d)
{
	gridPointUint32_t dims, idxLo;
	fpv_t             *velxP = gridPatch_getVarDataHandle(d->patch, 0);
	fpv_t             *velyP = gridPatch_getVarDataHandle(d->patch, 1);
	fpv_t             *velzP = gridPatch_getVarDataHandle(d->patch, 2);
	const double      dx     = d->data->boxsizeInMpch / d->fullDims[0];

	gridPatch_getIdxLo(d->patch, idxLo);
	gridPatch_getDims(d->patch, dims);

	printf("Patch idxLo: (%u,%u,%u)\n", idxLo[0], idxLo[1], idxLo[2]);
	printf("Patch dims:  (%u,%u,%u)\n", dims[0], dims[1], dims[2]);

	gridPointUint32_t p;
	uint64_t          i = 0;
	for (p[2] = idxLo[2]; p[2] < idxLo[2] + dims[2]; p[2]++) {
		for (p[1] = idxLo[1]; p[1] < idxLo[1] + dims[1]; p[1]++) {
			for (p[0] = idxLo[0]; p[0] < idxLo[0] + dims[0]; p[0]++) {
				d->vel[i * 3]     = velxP[i];
				d->vel[i * 3 + 1] = velyP[i];
				d->vel[i * 3 + 2] = velzP[i];
				d->pos[i * 3]     = (fpv_t)( (p[0] + .5) * dx );
				d->pos[i * 3 + 1] = (fpv_t)( (p[1] + .5) * dx );
				d->pos[i * 3 + 2] = (fpv_t)( (p[2] + .5) * dx );
				if (d->mode->useLongIDs) {
					( (uint64_t *)(d->id) )[i] = lIdx_fromCoord3d(p,
					                                              d->fullDims);
				} else {
					( (uint32_t *)(d->id) )[i] = lIdx_fromCoord3d(p,
					                                              d->fullDims);
				}
				i++;
			}
		}
	}
} // generateICsCore_vel2pos

extern void
generateICsCore_vel2pos(generateICsCore_const_t d)
{
#define SCALE(d, k)                              \
	(fmod(d->pos[k] + d->data->vFact * d->vel[k], \
	     d->data->boxsizeInMpch) * d->data->posFactor)
	for (uint64_t i = 0; i < d->numParticles; i++) {
		d->pos[i * 3]     += (fpv_t)(d->data->boxsizeInMpch);
		d->pos[i * 3]      = (fpv_t)( SCALE( d, (i * 3) ) );
		d->pos[i * 3 + 1] += (fpv_t)(d->data->boxsizeInMpch);
		d->pos[i * 3 + 1]  = (fpv_t)( SCALE( d, (i * 3 + 1) ) );
		d->pos[i * 3 + 2] += (fpv_t)(d->data->boxsizeInMpch);
		d->pos[i * 3 + 2]  = (fpv_t)( SCALE( d, (i * 3 + 2) ) );
	}
#undef SCALE
}

extern void
generateICsCore_convertVel(generateICsCore_const_t d)
{
	const fpv_t fac = d->data->velFactor / sqrt(d->data->aInit);
	for (uint64_t i = 0; i < d->numParticles; i++) {
		d->vel[i * 3]     *= fac;
		d->vel[i * 3 + 1] *= fac;
		d->vel[i * 3 + 2] *= fac;
	}
}

/*--- Implementations of local functions --------------------------------*/
