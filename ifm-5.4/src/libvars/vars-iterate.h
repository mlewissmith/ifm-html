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
  @brief Iteration functions and macros.
  @ingroup iterate
*/

#ifndef VARS_ITERATE_H
#define VARS_ITERATE_H

#include <vars-type.h>

/*! No. of iteration values per data type */
#ifndef V_MAXVALS
#define V_MAXVALS 2
#endif

/*!
  @brief Iterate over an object.

  This macro is a convenience which performs iteration over a Vars object
  using an iterator of type @ref viter.  It calls v_iter() once and then
  starts a while loop using v_next().

  Because of the single v_iter() call, be careful when nesting uses of this
  macro.  For example,

  @verbatim
  v_iterate(hash, i)
      v_iterate(vh_iter_pval(i), j)
          ...
  @endverbatim

  will not do what you expect: the second while loop will not be inside the
  first.  Use @c {} brackets, like this:

  @verbatim
  v_iterate(hash, i) {
      v_iterate(vh_iter_pval(i), j) {
          ...
      }
  }
  @endverbatim

  @bug This macro doesn't work correctly when nested without @c {}
  brackets.
*/
#define v_iterate(obj, iter)    v_iter(obj, &iter); while (v_next(&iter))

/*! @brief Iteration object. */
#define v_iter_object(iter)     iter.object

/*! @brief Iteration counter value. */
#define v_iter_count(iter)      iter.count

/*! @brief Iteration scalar value. */
#define v_iter_val(iter)        (vscalar *) iter.pval[0]

/*! @brief Iteration scalar integer value. */
#define v_iter_ival(iter)       vs_iget(v_iter_val(iter))

/*! @brief Iteration scalar float value. */
#define v_iter_fval(iter)       vs_fget(v_iter_val(iter))

/*! @brief Iteration scalar double value. */
#define v_iter_dval(iter)       vs_dget(v_iter_val(iter))

/*! @brief Iteration scalar string value. */
#define v_iter_sval(iter)       vs_sget(v_iter_val(iter))

/*! @brief Iteration scalar string value reference. */
#define v_iter_svalref(iter)    vs_sgetref(v_iter_val(iter))

/*! @brief Iteration scalar string value copy. */
#define v_iter_svalcopy(iter)   vs_sgetcopy(v_iter_val(iter))

/*! @brief Iteration scalar pointer value. */
#define v_iter_pval(iter)       vs_pget(v_iter_val(iter))

/*! @brief Iterator object type (read-only). */
typedef struct v_iter {
    /*! Object iterated over.  Use v_iter_object() to access this. */
    void *object;

    /*! Iteration counter.  Use v_iter_count() to access this. */
    int count;

    /*! Integer iteration position. */
    int ipos;

    /*! Pointer iteration position. */
    void *ppos;

    /*! Integer iteration values. */
    int ival[V_MAXVALS];

    /*! Float iteration values. */
    float fval[V_MAXVALS];

    /*! Double iteration values. */
    double dval[V_MAXVALS];

    /*! String iteration values. */
    char *sval[V_MAXVALS];

    /*! Pointer iteration values. */
    void *pval[V_MAXVALS];
} viter;

#ifdef __cplusplus
extern "C" {
#endif

extern void v_iter(void *object, viter *iter);
extern int v_iterable(vtype *t);
extern int v_next(viter *iter);

#ifdef __cplusplus
}
#endif

#endif
