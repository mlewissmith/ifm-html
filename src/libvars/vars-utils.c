/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup util Utility functions
*/

/*!
  @defgroup util_string String functions
  @ingroup util
*/

/*!
  @defgroup util_compress Compression functions
  @ingroup util
*/

/*!
  @defgroup util_math Mathematical functions
  @ingroup util
*/

/*!
  @defgroup util_error Error and warning functions
  @ingroup util
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-debug.h"
#include "vars-list.h"
#include "vars-macros.h"
#include "vars-memory.h"
#include "vars-utils.h"

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

/* Hook functions */
static void (*fatal_hook)(char *msg) = NULL;
static vlist *exception_hooks = NULL;

/* Combination state variables */
static int *c_flags = NULL;
static int *c_result = NULL;
static int c_total = 0;
static int c_size = 0;
static int c_num = 0;

/* Permutation state variables */
static int *p_count = NULL;
static int *p_result = NULL;
static int p_dcount = 0;
static int p_total = 0;
static int p_num = 0;

/*!
  @brief   Chop the last character from a string and return it.
  @ingroup util_string
  @param   str String.
  @return  Character removed.
*/
int
v_chop(char *str)
{
    int len = strlen(str);
    int c = EOF;

    if (len > 0) {
        c = str[len - 1];
        str[len - 1] = '\0';
    }

    return c;
}

/*!
  @brief   Iterate over all combinations of a set of integers.
  @ingroup util_math
  @param   total Total no. of integers.
  @param   num No. of integers to choose.

  If \c num is negative, it means all but \c -num integers of the set.

  This function initializes the iteration.  Use v_combine_next() to return
  all the combinations.
*/
void
v_combine(int total, int num)
{
    int i;

    if (total <= 0)
        v_fatal("v_combine(): invalid no. of integers");

    V_DEALLOC(c_result);
    c_result = V_ALLOC(int, total);

    V_DEALLOC(c_flags);
    c_flags = V_ALLOC(int, total);
    for (i = 0; i < total; i++)
        c_flags[i] = 0;

    if (num < 0)
        num += total;

    if (num <= 0)
        num = total;

    c_num = num;
    c_total = total;
    c_size = 0;
}

/*!
  @brief   Return next combination of a set of integers.
  @ingroup util_math
  @return  Combination of integers (pointer to internal buffer).
  @retval  NULL if no more combinations.
  @see     v_combine()
*/
int *
v_combine_next(void)
{
    int i, j;

    if (c_flags == NULL)
        return NULL;

    /* Find next combination of given length */
    while (1) {
        for (i = 0; i < c_total && c_flags[i]; c_flags[i++] = 0)
            c_size--;

        if (i < c_total) {
            c_flags[i] = 1;
            c_size++;
        } else {
            V_DEALLOC(c_flags);
            return NULL;
        }

        if (c_size == c_num)
            break;
    }

    /* Return it */
    for (i = 0, j = 0; i < c_total; i++)
        if (c_flags[i])
            c_result[j++] = i;

    return c_result;
}

/*!
  @brief   Compress some data.
  @ingroup util_compress
  @param   data Data to compress.
  @param   size Size of the data.
  @param[out]   csize Pointer to compressed size.
  @return  Compressed data.
  @retval  NULL if it failed.

  Use v_uncompress() to uncompress the data again.  It's up to you to store
  the compressed and uncompressed sizes for passing to that function.
*/
unsigned char *
v_compress(unsigned char *data, unsigned long size, unsigned long *csize)
{
#ifdef HAVE_LIBZ
    unsigned char *cdata;

    *csize = (unsigned long) (size * 1.001 + 12);
    cdata = V_ALLOC(unsigned char, *csize);

    if (compress(cdata, csize, data, size) != Z_OK) {
        V_DEALLOC(cdata);
        return NULL;
    }

    return cdata;
#else
    v_unavailable("v_compress()");
    return NULL;
#endif
}

/*!
  @brief   Give an error message and die.
  @ingroup util_error
  @param   fmt Format string.
*/
void
v_die(char *fmt, ...)
{
    V_BUF_DECL;
    char *msg;

    V_BUF_FMT(fmt, msg);
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

/*!
  @brief   Give an exception.
  @ingroup util_error
  @param   fmt Format string.

  The default action is to do nothing.  This can be changed using
  v_exception_hook().
*/
void
v_exception(char *fmt, ...)
{
    void (*func)(char *msg);
    V_BUF_DECL;
    viter iter;
    char *msg;

    if (exception_hooks != NULL) {
        V_BUF_FMT(fmt, msg);
        v_iterate(exception_hooks, iter)
            if ((func = vl_iter_pval(iter)) != NULL)
                func(msg);
    }
}

/*!
  @brief   Add hook function for exceptions.
  @ingroup util_error
  @param   func Function.
  @see     v_exception()
*/
void
v_exception_hook(void (*func)(char *msg))
{
    if (exception_hooks == NULL)
        exception_hooks = vl_create();

    vl_ppush(exception_hooks, func);
}

/*!
  @brief   Give a fatal error and die.
  @ingroup util_error
  @param   fmt Format string.

  The default action is to print the message to \c stderr.  This can be
  changed using v_fatal_hook().
*/
void
v_fatal(char *fmt, ...)
{
    V_BUF_DECL;
    char *msg;

    V_BUF_FMT(fmt, msg);

    if (fatal_hook == NULL)
        fprintf(stderr, "Vars fatal: %s\n", msg);
    else
        fatal_hook(msg);

    exit(2);
}

/*!
  @brief   Set hook function for fatal errors.
  @ingroup util_error
  @param   func Function (or \c NULL to unset it).
  @see     v_fatal()
*/
void
v_fatal_hook(void (*func)(char *msg))
{
    fatal_hook = func;
}

/* Fast conversion of pointer to hex string */
char *
v_hexstring(void *ptr, char *buf)
{
    static char *letters = "0123456789abcdef";
    unsigned long num = (unsigned long) ptr;
    static char hbuf[V_HEXSTRING_SIZE];
    char *cp;

    if (buf == NULL)
        buf = hbuf;

    cp = &buf[V_HEXSTRING_SIZE - 1];
    *cp = '\0';

    while (num) {
        *--cp = letters[num % 16];
        num /= 16;
    }

    *--cp = 'x';
    *--cp = '0';

    return cp;
}

/*!
  @brief   Iterate over all permutations of a set of integers.
  @ingroup util_math
  @param   total Total no. of integers.
  @param   num No. of integers to choose.

  If \c num is negative, it means all but \c -num integers of the set.

  This function initializes the iteration.  Use v_permute_next() to return
  all the permutations.
*/
void
v_permute(int total, int num)
{
    int i;

    if (total <= 0)
        v_fatal("v_permute(): invalid no. of integers");

    if (num < 0)
        num += total;

    if (num <= 0)
        num = total;

    V_DEALLOC(p_result);
    p_result = V_ALLOC(int, total);

    V_DEALLOC(p_count);
    p_count = V_ALLOC(int, total);

    for (i = 0; i < total; i++) {
        p_result[i] = 0;
        p_count[i] = 0;
    }

    p_count[0] = p_num = num;
    p_total = total;
    p_dcount = 1;
}

/*!
  @brief   Return next permutation of a set of integers.
  @ingroup util_math
  @return  Permutation of integers (pointer to internal buffer).
  @retval  NULL if no more permutations.
  @see     v_permute()
*/
int *
v_permute_next(void)
{
    int i, prev, next;

    if (p_count == NULL)
        return NULL;

    while (1) {
        /* Go to next number */
        for (i = p_num - 1; i >= 0; i--) {
            /* Increment number */
            prev = p_result[i];

            if ((next = prev + 1) == p_total)
                next = 0;

            p_result[i] = next;

            /* Update count of duplicates */
            if (--p_count[prev] == 1)
                p_dcount--;

            if (p_count[next]++ == 1)
                p_dcount++;

            /* Check for next number */
            if (next > 0)
                break;
        }

        /* Check for end */
        if (next == 0) {
            V_DEALLOC(p_count);
            return NULL;
        }

        /* If permutation, return it */
        if (p_dcount == 0)
            return p_result;
    }
}

/* Give an unavailable-function error and die */
void
v_unavailable(char *name)
{
    v_fatal("function '%s' is not available on this system", name);
}

/*!
  @brief   Uncompress some data.
  @ingroup util_compress
  @param   cdata Compressed data.
  @param   csize Size of compressed data (as returned by v_compress()).
  @param   size Uncompressed size (as passed to v_compress()).
  @return  Uncompressed data
  @retval  NULL if it failed.

  Uncompresses data that was compressed using v_compress().
*/
unsigned char *
v_uncompress(unsigned char *cdata, unsigned long csize, unsigned long size)
{
#ifdef HAVE_LIBZ
    unsigned char *data;

    data = V_ALLOC(unsigned char, size);

    if (uncompress(data, &size, cdata, csize) != Z_OK) {
        V_DEALLOC(data);
        return NULL;
    }

    return data;
#else
    v_unavailable("v_uncompress()");
    return NULL;
#endif
}

/*!
  @brief   Write a warning message to \c stderr.
  @ingroup util_error
  @param   fmt Format string.
*/
void
v_warn(char *fmt, ...)
{
    V_BUF_DECL;
    char *msg;

    V_BUF_FMT(fmt, msg);
    fprintf(stderr, "%s\n", msg);
}

/* Give an internal warning message */
void
v_warn_internal(char *fmt, ...)
{
    V_BUF_DECL;
    char *msg;

    V_BUF_FMT(fmt, msg);
    fprintf(stderr, "Vars warning: %s\n", msg);
}
