// Copyright (C) 2010, Steffen Knollmann
// Released under the terms of the GNU General Public License version 3.
// This file is part of `ginnungagap'.

#include "../../version.h"
#include "../../config.h"
#include <stdio.h>
#include <stdlib.h>
#include "parseCmdline.h"


int
main(int argc, char **argv)
{
	parseCmdline(argc, argv);

	return EXIT_SUCCESS;
}
