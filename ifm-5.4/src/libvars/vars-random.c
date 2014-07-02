/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup util_random Random numbers
  @ingroup util

  These functions are available for producing various sorts of random
  numbers.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include "vars-config.h"
#include "vars-random.h"

/* Required size of unsigned long */
#define ULONG	4

/* Constants & alternatives */
#define NK	37		/* 21 */
#define NJ	24		/*  6 */
#define NV	14		/*  8 */

/* Initialisation flag */
static int initrand = 0;

/* Random number array & pointer */
static unsigned long rnb[NK];
static int ptr = 0;

static unsigned long borrow = 0L;

/*!
  @brief   Initialise random number generator.
  @ingroup util_random
  @param   seed Random seed.

  If a seed of zero is given, then a seed is chosen based on the current
  time and process ID.
*/
void
v_seed(unsigned long seed)
{
    int i, in, errno_saved = errno;
    unsigned long m = 1L;

    initrand = 1;

    /* Portability test */
    if (sizeof(unsigned long) < ULONG) {
        v_exception("v_seed(): unsigned long must be >= %d bytes", ULONG);
        return;
    }

    /* Initialize seed if required */
    if (seed == 0) {
        char tbuf[30], rev[30];
        int pos = 0;

        sprintf(tbuf, "%ld", time(NULL));
        for (i = strlen(tbuf) - 1; i >= 0; i--)
            rev[pos++] = tbuf[i];

        rev[pos] = '\0';
        sscanf(rev, "%ld", &seed);
        seed ^= getpid();
    }

    /* Fill initialisation vector */
    borrow = 0L;
    ptr = 0;
    rnb[0] = seed;

    for (i = 1; i < NK; i++) {
	in = (NV * i) % NK;
	rnb[in] = m;
	m = seed - m;
	seed = rnb[in];
    }

    /* Warm up the generator */
    for (i = 0; i < 1000; i++)
	v_random();

    /* Restore errno */
    errno = errno_saved;
}

/*!
  @brief   Return a random integer.
  @ingroup util_random
  @return  Random integer.
  @see     v_randint(), v_randprob(), v_randreal()

  If the random number seed has not yet been set, then it is set by calling
  v_seed(0).
*/
unsigned long
v_random(void)
{
    int i, k, errno_saved = errno;
    unsigned long pdiff, t;

    /* Initialise seed if required */
    if (!initrand)
        v_seed(0);

    if (++ptr == NK)
        ptr = 0;
    else
	return rnb[ptr];

    for (i = 0, k = NK - NJ; i < NK; i++, k++) {
	/* Calculate next NK values */
	if (k == NK)
	    k = 0;
	t = rnb[k];

	/* Clever bit */
	pdiff = t - rnb[i] - borrow;

	if (pdiff < t)
	    borrow = 0;
	else if (pdiff > t)
	    borrow = 1;

	rnb[i] = pdiff;
    }

    /* Restore errno */
    errno = errno_saved;

    return rnb[0];
}

/*!
  @brief   Return a normally distributed number.
  @ingroup util_random
  @param   mean Mean value.
  @param   sd Standard deviation.
  @return  Value.
*/
double
v_normal(double mean, double sd)
{
    double total = 0.0;
    int i;

    for (i = 0; i < 12; i++)
        total += v_randprob();

    return mean + (total - 6.0) * sd;
}

/*!
  @brief   Return a Poisson-distributed number.
  @ingroup util_random
  @param   expected Expected value.
  @return  Value.
*/
int
v_poisson(double expected)
{
    double e, x;
    int n = 0;

    e = exp(-expected);
    x = v_randprob();
    while (x > e) {
        x *= v_randprob();
        n++;
    }

    return n;
}
