/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @file
  @brief Random number functions and macros.
  @ingroup util_random
*/

#ifndef VARS_RANDOM_H
#define VARS_RANDOM_H

#ifndef V_MAXRAND
#define V_MAXRAND 100000000
#endif

/*! @brief Return a random integer twixt min and max. */
#define v_randint(min, max) \
        ((min) + (v_random() % ((max) - (min) + 1)))

/*! @brief Return a random real twixt 0 and 1. */
#define v_randprob() \
        ((double) v_randint(0, V_MAXRAND - 1) / (V_MAXRAND - 1))

/*! @brief Return a random real twixt min and max. */
#define v_randreal(min, max) \
        ((min) + (v_randprob() * ((max) - (min))))

#ifdef __cplusplus
extern "C" {
#endif

extern void v_seed(unsigned long seed);
extern unsigned long v_random(void);
extern double v_normal(double mean, double sd);
extern int v_poisson(double expected);

#ifdef __cplusplus
}
#endif

#endif
