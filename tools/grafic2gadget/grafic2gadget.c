// Copyright (C) 2010, Steffen Knollmann
// Released under the terms of the GNU General Public License version 3.
// This file is part of `ginnungagap'.


/*--- Includes ----------------------------------------------------------*/
#include "grafic2gadgetConfig.h"
#include "grafic2gadget.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include "../../src/libcosmo/cosmo.h"
#include "../../src/libcosmo/cosmoModel.h"
#include "../../src/libutil/xmem.h"
#include "../../src/libutil/xstring.h"
#include "../../src/libutil/grafic.h"
#include "../../src/libutil/gadget.h"
#include "../../src/libutil/xfile.h"
#include "../../src/libutil/diediedie.h"


/*--- Implemention of main structure ------------------------------------*/
#include "grafic2gadget_adt.h"


/*--- Prototypes of local functions -------------------------------------*/
static void
local_checkOutputName(const char *outName, bool force);

static void
local_getFactors(grafic_t     grafic,
                 uint32_t     np[3],
                 double       *dx,
                 double       *boxsize,
                 double       *vFact,
                 double       *aInit,
                 cosmoModel_t *model);

static gadgetHeader_t
local_getBaseHeader(gadget_t     gadget,
                    double       boxsize,
                    double       aInit,
                    cosmoModel_t model,
                    uint32_t     np[3]);

static void
local_getSlabNumbers(int i,
                     int numFiles,
                     int range,
                     int *numSlabStart,
                     int *numSlabEnd,
                     int *numSlabs);

static void
local_initposid(float    *pos,
                uint32_t *id,
                uint32_t np[3],
                int      zStart,
                int      zEnd,
                double   dx);

static void
local_vel2pos(float    *vel,
              float    *pos,
              uint64_t num,
              double   boxsize,
              double   vFact);

static void
local_convertVel(float *vel, uint64_t num, double aInit);


/*--- Implementations of exported functios ------------------------------*/
extern grafic2gadget_t
grafic2gadget_new(const char *graficFileNameVx,
                  const char *graficFileNameVy,
                  const char *graficFileNameVz,
                  const char *outputFileStem,
                  int        numOutFiles,
                  bool       force)
{
	grafic2gadget_t g2g;

	assert(graficFileNameVx != NULL);
	assert(graficFileNameVy != NULL);
	assert(graficFileNameVz != NULL);
	assert(outputFileStem != NULL);
	assert(numOutFiles > 0);

	g2g                   = xmalloc(sizeof(struct grafic2gadget_struct));
	g2g->graficFileNameVx = xstrdup(graficFileNameVx);
	g2g->graficFileNameVy = xstrdup(graficFileNameVy);
	g2g->graficFileNameVz = xstrdup(graficFileNameVz);
	g2g->numGadgetFiles   = numOutFiles;
	g2g->gadgetFileStem   = xstrdup(outputFileStem);

	return g2g;
}

extern void
grafic2gadget_del(grafic2gadget_t *g2g)
{
	assert(g2g != NULL);
	assert(*g2g != NULL);

	xfree((*g2g)->graficFileNameVx);
	xfree((*g2g)->graficFileNameVy);
	xfree((*g2g)->graficFileNameVz);
	xfree((*g2g)->gadgetFileStem);
	xfree(*g2g);
	*g2g = NULL;
}

extern void
grafic2gadget_run(grafic2gadget_t g2g)
{
	grafic_t       gvx, gvy, gvz;
	uint32_t       np[3];
	uint64_t       numTotal, numPlane;
	double         dx, boxsize, vFact, aInit;
	cosmoModel_t   model;
	gadget_t       gadget;
	gadgetHeader_t baseHeader;

	assert(g2g != NULL);

	gvx    = grafic_newFromFile(g2g->graficFileNameVx, false);
	gvy    = grafic_newFromFile(g2g->graficFileNameVy, false);
	gvz    = grafic_newFromFile(g2g->graficFileNameVz, false);

	gadget = gadget_new(g2g->gadgetFileStem, g2g->numGadgetFiles);

	local_getFactors(gvx, np, &dx, &boxsize, &vFact, &aInit, &model);
	baseHeader = local_getBaseHeader(gadget, boxsize, aInit, model, np);

	numTotal   = np[0] * np[1] * np[2];
	numPlane   = np[0] * np[1];
	for (int i = 0; i < g2g->numGadgetFiles; i++) {
		int            numSlabStart, numSlabEnd, numSlabs;
		int64_t        numLocal;
		float          *vel, *pos;
		uint32_t       *id;
		uint32_t       npLocal[6] = {0, 0, 0, 0, 0, 0};
		gadgetHeader_t myHeader;

		local_getSlabNumbers(i, g2g->numGadgetFiles, np[2],
		                     &numSlabStart, &numSlabEnd, &numSlabs);
		myHeader   = gadgetHeader_clone(baseHeader);
		numLocal   = numSlabs * numPlane;
		npLocal[1] = numLocal;
		gadgetHeader_setNp(myHeader, npLocal);
		gadget_attachHeader(gadget, myHeader, i);

		vel = xmalloc(sizeof(float) * numLocal * 3);
		pos = xmalloc(sizeof(float) * numLocal * 3);
		id  = xmalloc(sizeof(uint32_t) * numLocal);

		for (int j = numSlabStart; j <= numSlabEnd; j++) {
			grafic_readSlab(gvx, vel + 3 * (j - numSlabStart) * numPlane,
			                GRAFIC_FORMAT_FLOAT, 3, j);
			grafic_readSlab(gvy,
			                vel + 3 * (j - numSlabStart) * numPlane + 1,
			                GRAFIC_FORMAT_FLOAT,
			                3,
			                j);
			grafic_readSlab(gvz,
			                vel + 3 * (j - numSlabStart) * numPlane + 2,
			                GRAFIC_FORMAT_FLOAT,
			                3,
			                j);
		}
		local_initposid(pos, id, np, numSlabStart, numSlabEnd, dx);
		local_vel2pos(vel, pos, numLocal, boxsize, vFact);
		local_convertVel(vel, numLocal, aInit);

		gadget_open(gadget, GADGET_MODE_WRITE, i);
		gadget_write(gadget, i, pos, vel, id);
		gadget_close(gadget, i);

		xfree(id);
		xfree(pos);
		xfree(vel);
	}

	gadgetHeader_del(&baseHeader);
	gadget_del(&gadget);
	cosmoModel_del(&model);
	grafic_del(&gvz);
	grafic_del(&gvy);
	grafic_del(&gvx);
} /* grafic2gadget_run */

/*--- Implementations of local functions --------------------------------*/
static void
local_getFactors(grafic_t     grafic,
                 uint32_t     np[3],
                 double       *dx,
                 double       *boxsize,
                 double       *vFact,
                 double       *aInit,
                 cosmoModel_t *model)
{
	double error, adot, growthVel;
	float  h0;

	grafic_getSize(grafic, np);
	h0       = grafic_getH0(grafic);
	*dx      = grafic_getDx(grafic);
	*aInit   = grafic_getAstart(grafic);
	*dx     *= 0.01 * h0;
	assert(np[0] == np[1] && np[0] == np[2]);
	*boxsize = *dx * np[0];

	*model   = cosmoModel_new();
	cosmoModel_setOmegaLambda0(*model, grafic_getOmegav(grafic));
	cosmoModel_setOmegaMatter0(*model, grafic_getOmegam(grafic));
	cosmoModel_setSmallH(*model, 0.01 * h0);

	adot      = cosmoModel_calcADot(*model, *aInit);
	growthVel = cosmoModel_calcDlnGrowthDlna(*model, *aInit, &error);
	*vFact    = 1. / (adot * 100. * growthVel);
}

static gadgetHeader_t
local_getBaseHeader(gadget_t     gadget,
                    double       boxsize,
                    double       aInit,
                    cosmoModel_t model,
                    uint32_t     np[3])
{
	uint64_t       npall[6];
	double         massarr[6];
	double         omegaMatter0 = cosmoModel_getOmegaMatter0(model);
	int            numFiles     = gadget_getNumFiles(gadget);
	gadgetHeader_t header       = gadgetHeader_new();

	for (int i = 0; i < 6; i++) {
		npall[i]   = 0;
		massarr[i] = 0;
	}

	npall[1]    = np[0];
	npall[1]   *= np[1];
	npall[1]   *= np[2];
	massarr[1]  = boxsize * boxsize * boxsize * omegaMatter0 / (npall[1]);
	massarr[1] *= COSMO_RHO_CRIT0 * 1e-10;


	gadgetHeader_setMassArr(header, massarr);
	gadgetHeader_setExpansion(header, aInit);
	gadgetHeader_setNall(header, npall);
	gadgetHeader_setNumFiles(header, numFiles);
	gadgetHeader_setBoxsize(header, boxsize);
	gadgetHeader_setOmega0(header, cosmoModel_getOmegaMatter0(model));
	gadgetHeader_setOmegaLambda(header, cosmoModel_getOmegaLambda0(model));
	gadgetHeader_setHubbleParameter(header, cosmoModel_getSmallH(model));

	return header;
}

static void
local_getSlabNumbers(int i,
                     int numFiles,
                     int range,
                     int *numSlabStart,
                     int *numSlabEnd,
                     int *numSlabs)
{
	int slabsPerTask = range / numFiles;
	int extra        = range % numFiles;

	if (i < extra) {
		*numSlabs     = slabsPerTask + 1;
		*numSlabStart = i * (*numSlabs);
		*numSlabEnd   = (i + 1) * (*numSlabs) - 1;
	} else {
		*numSlabs     = slabsPerTask;
		*numSlabStart = extra * (slabsPerTask + 1)
		                + (i - extra) * slabsPerTask;
		*numSlabEnd   = *numSlabStart + slabsPerTask - 1;
	}
}

static void
local_initposid(float    *pos,
                uint32_t *id,
                uint32_t np[3],
                int      zStart,
                int      zEnd,
                double   dx)
{
#ifdef _OPENMP
#  pragma omp parallel for
#endif
	for (int k = zStart; k <= zEnd; k++) {
		for (int j = 0; j < np[1]; j++) {
			for (int i = 0; i < np[0]; i++) {
				size_t idx = i + j * np[0] + (k - zStart) * np[1] * np[0];
				id[idx]          = i + j * np[0] + k * np[1] * np[0];
				pos[idx * 3]     = (i + .5) * dx;
				pos[idx * 3 + 1] = (j + .5) * dx;
				pos[idx * 3 + 2] = (k + .5) * dx;
			}
		}
	}
}

static void
local_vel2pos(float    *vel,
              float    *pos,
              uint64_t num,
              double   boxsize,
              double   vFact)
{
#ifdef _OPENMP
#  pragma omp parallel for
#endif
	for (uint64_t i = 0; i < num; i++) {
		pos[i * 3]     += boxsize;
		pos[i * 3 + 1] += boxsize;
		pos[i * 3 + 2] += boxsize;
		pos[i * 3]      = fmod(pos[i * 3] + vFact * vel[i * 3],
		                       boxsize);
		pos[i * 3 + 1]  = fmod(pos[i * 3 + 1] + vFact * vel[i * 3 + 1],
		                       boxsize);
		pos[i * 3 + 2]  = fmod(pos[i * 3 + 2] + vFact * vel[i * 3 + 2],
		                       boxsize);
	}
}

static void
local_convertVel(float *vel, uint64_t num, double aInit)
{
	double fac = 1. / (sqrt(aInit));

#ifdef _OPENMP
#  pragma omp parallel for
#endif
	for (uint64_t i = 0; i < num; i++) {
		vel[i * 3]     *= fac;
		vel[i * 3 + 1] *= fac;
		vel[i * 3 + 2] *= fac;
	}
}