/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Configuration functions */

#include <math.h>
#include "vars-config.h"

/* Replacement for missing drem() */
double
v_drem(double x, double y)
{
    return x - y * round(x / y);
}
