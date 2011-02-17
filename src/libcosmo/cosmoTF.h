// Copyright (C) 2010, 2011, Steffen Knollmann
// Released under the terms of the GNU General Public License version 3.
// This file is part of `ginnungagap'.

#ifndef COSMOTF_H
#define COSMOTF_H


/*--- Includes ----------------------------------------------------------*/
#include "cosmo_config.h"
#include "../libutil/parse_ini.h"
#include <stdint.h>


/*--- Exported types ----------------------------------------------------*/
typedef enum {
	COSMOTF_TYPE_EISENSTEINHU1998,
	COSMOTF_TYPE_SCALEFREE,
	COSMOTF_TYPE_ANATOLY2000
} cosmoTF_t;


/*--- Prototypes of exported functions ----------------------------------*/
extern cosmoTF_t
cosmoTF_getTypeFromIni(parse_ini_t ini, const char *sectionName);

extern void
cosmoTF_eisensteinHu1998(double   omegaMatter0,
                         double   omegaBaryon0,
                         double   hubble,
                         double   tempCMB,
                         uint32_t numPoints,
                         double   *k,
                         double   *T);

extern void
cosmoTF_scaleFree(uint32_t numPoints,
                  double   *k,
                  double   *T);


#endif
