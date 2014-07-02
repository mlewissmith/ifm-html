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
  @brief Priority queue functions and macros.
  @ingroup queue
*/

#ifndef VARS_QUEUE_H
#define VARS_QUEUE_H

#include <vars-scalar.h>

/*! @brief Abort if a pointer is not a queue. */
#define VQ_CHECK(ptr)           V_CHECK(ptr, vq_check, "QUEUE")

/*! @brief Add an integer value to a queue. */
#define vq_istore(q, v, p)      vq_store(q, vs_icreate(v), p)

/*! @brief Add a float value to a queue. */
#define vq_fstore(q, v, p)      vq_store(q, vs_fcreate(v), p)

/*! @brief Add a double value to a queue. */
#define vq_dstore(q, v, p)      vq_store(q, vs_dcreate(v), p)

/*! @brief Add a string value to a queue. */
#define vq_sstore(q, v, p)      vq_store(q, vs_screate(v), p)

/*! @brief Add a pointer value to a queue. */
#define vq_pstore(q, v, p)      vq_store(q, vs_pcreate(v), p)

/*! @brief Remove and return an integer value from a queue. */
#define vq_iget(q)              vs_free_iget(vq_get(q))

/*! @brief Remove and return a float value from a queue. */
#define vq_fget(q)              vs_free_fget(vq_get(q))

/*! @brief Remove and return a double value from a queue. */
#define vq_dget(q)              vs_free_dget(vq_get(q))

/*! @brief Remove and return a string value from a queue. */
#define vq_sget(q)              vs_free_sget(vq_get(q))

/*! @brief Remove and return a pointer value from a queue. */
#define vq_pget(q)              vs_free_pget(vq_get(q))

/*! @brief Return the next integer value from a queue. */
#define vq_ihead(q)             vs_iget(vq_head(q))

/*! @brief Return the next float value from a queue. */
#define vq_fhead(q)             vs_fget(vq_head(q))

/*! @brief Return the next double value from a queue. */
#define vq_dhead(q)             vs_dget(vq_head(q))

/*! @brief Return the next string value from a queue. */
#define vq_shead(q)             vs_sget(vq_head(q))

/*! @brief Return the next pointer value from a queue. */
#define vq_phead(q)             vs_pget(vq_head(q))

/*! @brief Create or empty a queue. */
#define vq_init(q) if (q == NULL) q = vq_create(); else vq_empty(q)

/*! @brief Iteration priority value. */
#define vq_iter_priority(iter)  iter.dval[0]

/*! @brief Iteration scalar value. */
#define vq_iter_val(iter)       v_iter_val(iter)

/*! @brief Iteration scalar integer value. */
#define vq_iter_ival(iter)      v_iter_ival(iter)

/*! @brief Iteration scalar float value. */
#define vq_iter_fval(iter)      v_iter_fval(iter)

/*! @brief Iteration scalar double value. */
#define vq_iter_dval(iter)      v_iter_dval(iter)

/*! @brief Iteration scalar string value. */
#define vq_iter_sval(iter)      v_iter_sval(iter)

/*! @brief Iteration scalar string value reference. */
#define vq_iter_svalref(iter)   v_iter_svalref(iter)

/*! @brief Iteration scalar string value copy. */
#define vq_iter_svalcopy(iter)  v_iter_svalcopy(iter)

/*! @brief Iteration scalar pointer value. */
#define vq_iter_pval(iter)      v_iter_pval(iter)

/*! @brief Queue type. */
typedef struct v_queue vqueue;

#ifdef __cplusplus
extern "C" {
#endif

extern int vq_check(void *ptr);
extern vqueue *vq_copy(vqueue *q);
extern vqueue *vq_create(void);
extern vqueue *vq_create_size(int size);
extern vtype *vq_declare(void);
extern void vq_destroy(vqueue *q);
extern void vq_empty(vqueue *q);
extern int vq_freeze(vqueue *q, FILE *fp);
extern vscalar *vq_get(vqueue *q);
extern vscalar *vq_head(vqueue *q);
extern int vq_length(vqueue *q);
extern int vq_next(viter *iter);
extern void vq_print(vqueue *q, FILE *fp);
extern vqueue *vq_read(FILE *fp);
extern void vq_store(vqueue *q, vscalar *s, double priority);
extern vqueue *vq_thaw(FILE *fp);
extern int vq_traverse(vqueue *q, int (*func)(void *ptr));
extern int vq_write(vqueue *q, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
