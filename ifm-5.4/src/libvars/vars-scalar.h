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
  @brief Scalar functions and macros.
  @ingroup scalar
*/

#ifndef VARS_SCALAR_H
#define VARS_SCALAR_H

#include <vars-type.h>

/*! @brief Abort of a pointer is not a scalar. */
#define VS_CHECK(ptr)           V_CHECK(ptr, vs_check, "SCALAR")

/*! @brief Create a scalar with a double value. */
#define vs_dcreate(val)         vs_dstore(NULL, val)

/*! @brief Create a scalar with a float value. */
#define vs_fcreate(val)         vs_fstore(NULL, val)

/*! @brief Create a scalar with an integer value. */
#define vs_icreate(val)         vs_istore(NULL, val)

/*! @brief Create a scalar with a pointer value. */
#define vs_pcreate(val)         vs_pstore(NULL, val)

/*! @brief Create a scalar with a string value. */
#define vs_screate(val)         vs_sstore(NULL, val)

/*! @brief Return a copy of a scalar string value. */
#define vs_sgetcopy(val)        V_STRDUP(vs_sget(val))

/*! @brief Scalar variable types. */
enum v_stype {
    V_TYPE_INT,                 /*!< Integer */
    V_TYPE_FLOAT,               /*!< Float */
    V_TYPE_DOUBLE,              /*!< Double */
    V_TYPE_STRING,              /*!< String */
    V_TYPE_POINTER,             /*!< Pointer */
    V_TYPE_UNDEF,               /*!< Undefined */
    V_TYPE_NULL                 /*!< Null */
};

/*! @brief Scalar type. */
typedef struct v_scalar vscalar;

#ifdef __cplusplus
extern "C" {
#endif

extern int vs_casecmp(vscalar **s1, vscalar **s2);
extern int vs_check(void *ptr);
extern int vs_cmp(vscalar **s1, vscalar **s2);
extern vscalar *vs_copy(vscalar *s);
extern vscalar *vs_create(enum v_stype type);
extern vtype *vs_declare(void);
extern int vs_defined(vscalar *s);
extern void vs_destroy(vscalar *s);
extern double vs_dget(vscalar *s);
extern vscalar *vs_dstore(vscalar *s, double val);
extern int vs_equal(vscalar *s1, vscalar *s2);
extern float vs_fget(vscalar *s);
extern double vs_free_dget(vscalar *s);
extern float vs_free_fget(vscalar *s);
extern int vs_free_iget(vscalar *s);
extern void *vs_free_pget(vscalar *s);
extern char *vs_free_sget(vscalar *s);
extern int vs_freeze(vscalar *s, FILE *fp);
extern vscalar *vs_fstore(vscalar *s, float val);
extern int vs_iget(vscalar *s);
extern vscalar *vs_istore(vscalar *s, int val);
extern int vs_numcmp(vscalar **s1, vscalar **s2);
extern void *vs_pget(vscalar *s);
extern void vs_print(vscalar *s, FILE *fp);
extern vscalar *vs_pstore(vscalar *s, void *val);
extern vscalar *vs_read(FILE *fp);
extern vscalar *vs_scalar(enum v_stype type, ...);
extern char *vs_sget(vscalar *s);
extern char *vs_sget_buf(vscalar *s, char *sval);
extern char *vs_sgetref(vscalar *s);
extern vscalar *vs_sstore(vscalar *s, char *val);
extern vscalar *vs_sstore_len(vscalar *s, char *base, size_t len);
extern vscalar *vs_thaw(FILE *fp);
extern int vs_traverse(vscalar *s, int (*func)());
extern enum v_stype vs_type(vscalar *s);
extern void vs_undef(vscalar *s);
extern int vs_write(vscalar *s, FILE *fp);
extern int vs_xmldump(vscalar *s, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
