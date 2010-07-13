// Copyright (C) 2010, Steffen Knollmann
// Released under the terms of the GNU General Public License version 3.
// This file is part of `ginnungagap'.

#ifndef GINNUNGAGAP_ADT_H
#define GINNUNGAGAP_ADT_H


/*--- Includes ----------------------------------------------------------*/
#include "ginnungagapConfig.h"
#include "ginnungagapSetup.h"
#include "ginnungagapWN.h"
#include "../libcosmo/cosmoModel.h"
#include "../libcosmo/cosmoPk.h"
#include "../libgrid/gridRegular.h"
#include "../libgrid/gridRegularDistrib.h"
#include "../libgrid/gridRegularFFT.h"


/*--- Implemention of main structure ------------------------------------*/
struct ginnungagap_struct {
	ginnungagapSetup_t   setup;
	cosmoModel_t         model;
	cosmoPk_t            pk;
	ginnungagapWN_t      whiteNoise;
	gridRegular_t        grid;
	gridRegularDistrib_t gridDistrib;
	gridRegularFFT_t     gridFFT;
	int                  posOfDens;
	int                  rank;
	int                  size;
	int                  numThreads;
};


#endif