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
  @brief Useful macros
  @ingroup util
*/

#ifndef VARS_MACROS_H
#define VARS_MACROS_H

#ifndef V_PATHSEP
/*! @brief Path separator string. */
#define V_PATHSEP "/"
#endif

#ifndef V_TEMPDIR
/*! @brief Default temporary directory. */
#define V_TEMPDIR "/tmp"
#endif

/*! @brief Return the absolute value of a number. */
#define V_ABS(x)                ((x) > 0 ? (x) : -(x))

/*! @brief Return the minimum of two numbers. */
#define V_MIN(x, y)             ((x) < (y) ? (x) : (y))

/*! @brief Return the maximum of two numbers. */
#define V_MAX(x, y)             ((x) > (y) ? (x) : (y))

/*! @brief Return the nearest integer to a number. */
#define V_NINT(x)               ((int) ((x) + 0.5))

/*! @brief Numeric equivalent of strcmp(). */
#define V_CMP(a, b)             ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))

/*! @brief Compare strings for equality. */
#define V_STREQ(s1, s2)         (strcmp(s1, s2) == 0)

/*! @brief Compare strings for inequality. */
#define V_STRNEQ(s1, s2)        (strcmp(s1, s2) != 0)

/*! @brief Test if a string contains a substring. */
#define V_STRSUB(str, sub)      (strstr(str, sub) != NULL)

/*! @brief Constrain a value to a range. */
#define V_LIMIT(val, min, max)                                          \
        if      (val < (min)) val = (min);                              \
        else if (val > (max)) val = (max)

#endif
