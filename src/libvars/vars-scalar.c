/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup scalar Scalars
  @ingroup types

  Scalars are the basic unit of data in the Vars library. They are
  single-valued objects which can have values of type <tt>int</tt>,
  <tt>float</tt>, <tt>double</tt>, <tt>char *</tt> or <tt>void *</tt>. The
  last is the generic pointer type, so you can store a pointer to any
  object at all in a scalar.

  In practice, scalars are hardly ever used directly -- there are macros
  available which create them automatically from other types.
*/

/*!
  @defgroup scalar_create Creating and destroying scalars
  @ingroup scalar
*/

/*!
  @defgroup scalar_access Accessing scalar values
  @ingroup scalar
  
  These functions get the value of a scalar.  In most cases, conversion to
  the right type is done beforehand if required.
*/

/*!
  @defgroup scalar_modify Modifying scalars
  @ingroup scalar
*/

/*!
  @defgroup scalar_compare Scalar comparison
  @ingroup scalar
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "vars-config.h"
#include "vars-copy.h"
#include "vars-hash.h"
#include "vars-macros.h"
#include "vars-memory.h"
#include "vars-scalar.h"
#include "vars-system.h"
#include "vars-yaml.h"

#define FLOAT_FORMAT "%.12g"

/* Type definition */
struct v_scalar {
    struct v_header id;         /* Type marker */
    enum v_stype type;          /* Scalar type */
    union {
	int i;                  /* Integer value */
        float f;                /* Float value */
	double d;               /* Double value */
	char *s;                /* String value */
	void *p;                /* Pointer value */
    } v;
};

/* Scribble buffer */
static char buf[BUFSIZ];

/* Type variable */
vtype *vscalar_type = NULL;

/* Internal functions */
static int vs_yamldump(vscalar *s, FILE *fp);

/*!
  @brief Return case-independent string comparison of two scalars.
  @ingroup scalar_compare

  @param s1 Pointer to first scalar.
  @param s2 Pointer to second scalar.

  @retval -1 if \c s1 < \c s2
  @retval 0 if \c s1 = \c s2
  @retval 1 if \c s1 > \c s2

  This is designed for use in list sorting functions (vl_sort(),
  vl_insort(), etc.)
*/
int
vs_casecmp(vscalar **s1, vscalar **s2)
{
    static char buf2[BUFSIZ];

    if ((*s1)->type != V_TYPE_STRING) {
        vs_sget_buf(*s1, buf);
        if ((*s2)->type == V_TYPE_STRING) {
            return strcasecmp(buf, (*s2)->v.s);
        } else {
            vs_sget_buf(*s2, buf2);
            return strcasecmp(buf, buf2);
        }
    } else if ((*s2)->type != V_TYPE_STRING) {
        vs_sget_buf(*s2,  buf);
        return strcasecmp((*s1)->v.s, buf);
    }

    return strcasecmp((*s1)->v.s, (*s2)->v.s);
}

/* Check if pointer is a scalar */
int
vs_check(void *ptr)
{
    return (ptr != NULL && v_type(ptr) == vscalar_type);
}

/*!
  @brief Return string comparison of two scalars.
  @ingroup scalar_compare

  @param s1 Pointer to first scalar.
  @param s2 Pointer to second scalar.

  @retval -1 if \c s1 < \c s2
  @retval 0 if \c s1 = \c s2
  @retval 1 if \c s1 > \c s2

  This is designed for use in list sorting functions (vl_sort(),
  vl_insort(), etc.)
*/
int
vs_cmp(vscalar **s1, vscalar **s2)
{
    static char buf2[BUFSIZ];

    if ((*s1)->type != V_TYPE_STRING) {
        vs_sget_buf(*s1, buf);
        if ((*s2)->type == V_TYPE_STRING) {
            return strcmp(buf, (*s2)->v.s);
        } else {
            vs_sget_buf(*s2, buf2);
            return strcmp(buf, buf2);
        }
    } else if ((*s2)->type != V_TYPE_STRING) {
        vs_sget_buf(*s2,  buf);
        return strcmp((*s1)->v.s, buf);
    }

    return strcmp((*s1)->v.s, (*s2)->v.s);
}

/*!
  @brief Return a copy of a scalar.
  @ingroup scalar_create

  @param s Scalar to copy.

  @return The copy.
*/
vscalar *
vs_copy(vscalar *s)
{
    vscalar *vc;

    VS_CHECK(s);

    vc = vs_create(s->type);
    vc->v = s->v;

    if (s->type == V_TYPE_STRING)
	vc->v.s = V_STRDUP(s->v.s);
    else if (s->type == V_TYPE_POINTER && v_deepcopy)
        vc->v.p = v_copy(s->v.p);

    return vc;
}

/*!
  @brief Return a newly-created scalar.
  @ingroup scalar_create

  @param type Type of scalar to create.

  @return The scalar.
*/
vscalar *
vs_create(enum v_stype type)
{
    static vheader *id = NULL;
    vscalar *s;

    if (id == NULL) {
        vs_declare();
        id = v_header(vscalar_type);
    }

    s = V_ALLOC(vscalar, 1);

    s->id = *id;
    s->type = type;

    switch (type) {
    case V_TYPE_UNDEF:
    case V_TYPE_INT:
	s->v.i = 0;
	break;
    case V_TYPE_FLOAT:
        s->v.f = 0.0;
        break;
    case V_TYPE_DOUBLE:
	s->v.d = 0.0;
	break;
    case V_TYPE_STRING:
	s->v.s = NULL;
	break;
    case V_TYPE_POINTER:
	s->v.p = NULL;
	break;
    default:
        v_fatal("vs_create(): invalid scalar type");
	return NULL;
    }

    return s;
}

/* Declare scalar type */
vtype *
vs_declare(void)
{
    if (vscalar_type == NULL) {
        vscalar_type = v_create("SCALAR", "S");
        v_copy_func(vscalar_type, (void *(*)()) vs_copy);
        v_read_func(vscalar_type, (void *(*)()) vs_read);
        v_write_func(vscalar_type, vs_write);
        v_print_func(vscalar_type, vs_print);
        v_freeze_func(vscalar_type, vs_freeze);
        v_thaw_func(vscalar_type, (void *(*)()) vs_thaw);
        v_xmldump_func(vscalar_type, vs_xmldump);
        v_yamldump_func(vscalar_type, vs_yamldump);
        v_destroy_func(vscalar_type, vs_destroy);
        v_traverse_func(vscalar_type, vs_traverse);
    }

    return vscalar_type;
}

/* Return whether a scalar is defined */
int
vs_defined(vscalar *s)
{
    VS_CHECK(s);
    return (s->type != V_TYPE_UNDEF);
}

/*!
  @brief Deallocate a scalar.
  @ingroup scalar_create

  @param s Scalar to destroy.
*/
void
vs_destroy(vscalar *s)
{
    VS_CHECK(s);

    if (s->type == V_TYPE_STRING && !vh_interned(s->v.s))
	V_DEALLOC(s->v.s);

    V_DEALLOC(s);
}

/*!
  @brief Return double value of a scalar.
  @ingroup scalar_access

  @param s Scalar.

  @return Value.
*/
double
vs_dget(vscalar *s)
{
    double dval = 0.0;

    if (s == NULL)
	return dval;

    VS_CHECK(s);

    switch (s->type) {
    case V_TYPE_INT:
	dval = (double) s->v.i;
	break;
    case V_TYPE_FLOAT:
        dval = (double) s->v.f;
        break;
    case V_TYPE_DOUBLE:
	dval = s->v.d;
	break;
    case V_TYPE_STRING:
	sscanf(s->v.s, "%lf", &dval);
	break;
    case V_TYPE_POINTER:
        v_exception("vs_dget(): scalar is a pointer");
        break;
    }

    return dval;
}

/*!
  @brief Set a scalar to a double value.
  @ingroup scalar_modify

  @param s Scalar (or <tt>NULL</tt>).
  @param val Value to set.

  @return Scalar that was set.
*/
vscalar *
vs_dstore(vscalar *s, double val)
{
    if (s == NULL)
	s = vs_create(V_TYPE_DOUBLE);

    VS_CHECK(s);

    if (s->type == V_TYPE_STRING)
	V_DEALLOC(s->v.s);

    s->type = V_TYPE_DOUBLE;
    s->v.d = val;

    return s;
}

/*!
  @brief Return whether two scalars are equal.
  @ingroup scalar_compare

  @param s1 First scalar.
  @param s2 Second scalar.

  @return Yes or no.

  For equality, the scalars must have the same type and value.
*/
int
vs_equal(vscalar *s1, vscalar *s2)
{
    VS_CHECK(s1);
    VS_CHECK(s2);

    if (s1->type != s2->type)
        return 0;

    switch (s1->type) {
    case V_TYPE_INT:
        return (s1->v.i == s2->v.i);
    case V_TYPE_FLOAT:
        return (s1->v.f == s2->v.f);
    case V_TYPE_DOUBLE:
        return (s1->v.d == s2->v.d);
    case V_TYPE_STRING:
        return V_STREQ(s1->v.s, s2->v.s);
    case V_TYPE_POINTER:
        return (s1->v.p == s2->v.p);
    }

    return 1;
}

/*!
  @brief Return float value of a scalar.
  @ingroup scalar_access

  @param s Scalar.

  @return Value.
*/
float
vs_fget(vscalar *s)
{
    float fval = 0.0;

    if (s == NULL)
	return fval;

    VS_CHECK(s);

    switch (s->type) {
    case V_TYPE_INT:
	fval = (float) s->v.i;
	break;
    case V_TYPE_FLOAT:
        fval = s->v.f;
        break;
    case V_TYPE_DOUBLE:
	fval = (float) s->v.d;
	break;
    case V_TYPE_STRING:
	sscanf(s->v.s, "%f", &fval);
	break;
    case V_TYPE_POINTER:
        v_exception("vs_fget(): scalar is a pointer");
        break;
    }

    return fval;
}

/* Free a scalar and return its double value */
double
vs_free_dget(vscalar *s)
{
    double dval = vs_dget(s);

    if (s != NULL)
        vs_destroy(s);

    return dval;
}

/* Free a scalar and return its float value */
float
vs_free_fget(vscalar *s)
{
    float fval = vs_fget(s);

    if (s != NULL)
        vs_destroy(s);

    return fval;
}

/* Free a scalar and return its integer value */
int
vs_free_iget(vscalar *s)
{
    int ival = vs_iget(s);

    if (s != NULL)
        vs_destroy(s);

    return ival;
}

/* Free a scalar and return its pointer value */
void *
vs_free_pget(vscalar *s)
{
    void *pval = vs_pget(s);

    if (s != NULL)
        vs_destroy(s);

    return pval;
}

/* Free a scalar and return its string value */
char *
vs_free_sget(vscalar *s)
{
    char *sval = V_STRDUP(vs_sget_buf(s, buf));

    if (s != NULL)
        vs_destroy(s);

    return sval;
}

/* Freeze scalar to file */
int
vs_freeze(vscalar *s, FILE *fp)
{
    void *ptr;

    VS_CHECK(s);

    v_freeze_start(fp);

    switch(vs_type(s)) {
    case V_TYPE_INT:
        fprintf(fp, "%d", vs_iget(s));
        break;
    case V_TYPE_FLOAT:
    case V_TYPE_DOUBLE:
        fprintf(fp, FLOAT_FORMAT, vs_dget(s));
        break;
    case V_TYPE_STRING:
        if (!v_freeze_string(vs_sget_buf(s, buf), fp))
            return 0;
        break;
    case V_TYPE_POINTER:
        ptr = vs_pget(s);
        if (v_type(ptr) == vscalar_type) {
            fprintf(fp, "%s {\n", vscalar_type->name);
            v_push_indent();
            v_indent(fp);
            if (!vs_freeze(ptr, fp))
                return 0;
            fputc('\n', fp);
            v_pop_indent();
            v_indent(fp);
            fputc('}', fp);
        } else if (!v_freeze(ptr, fp)) {
            return 0;
        }
        break;
    case V_TYPE_UNDEF:
        fprintf(fp, "%s", V_TOKEN_UNDEF_STRING);
        break;
    }

    v_freeze_finish(fp);

    return 1;
}

/*!
  @brief Set a scalar to a float value.
  @ingroup scalar_modify

  @param s Scalar (or <tt>NULL</tt>).
  @param val Value to set.

  @return Scalar that was set.
*/
vscalar *
vs_fstore(vscalar *s, float val)
{
    if (s == NULL)
	s = vs_create(V_TYPE_FLOAT);

    VS_CHECK(s);

    if (s->type == V_TYPE_STRING)
	V_DEALLOC(s->v.s);

    s->type = V_TYPE_FLOAT;
    s->v.f = val;

    return s;
}

/*!
  @brief Return integer value of a scalar.
  @ingroup scalar_access

  @param s Scalar.

  @return Value.
*/
int
vs_iget(vscalar *s)
{
    int ival = 0;

    if (s == NULL)
	return ival;

    VS_CHECK(s);

    switch (s->type) {
    case V_TYPE_INT:
	ival = s->v.i;
	break;
    case V_TYPE_FLOAT:
        ival = (int) s->v.f;
        break;
    case V_TYPE_DOUBLE:
	ival = (int) s->v.d;
	break;
    case V_TYPE_STRING:
	sscanf(s->v.s, "%d", &ival);
	break;
    case V_TYPE_POINTER:
        v_exception("vs_iget(): scalar is a pointer");
        break;
    }

    return ival;
}

/*!
  @brief Set a scalar to an integer value.
  @ingroup scalar_modify

  @param s Scalar (or <tt>NULL</tt>).
  @param val Value to set.

  @return Scalar that was set.
*/
vscalar *
vs_istore(vscalar *s, int val)
{
    if (s == NULL)
	s = vs_create(V_TYPE_INT);

    VS_CHECK(s);

    if (s->type == V_TYPE_STRING)
	V_DEALLOC(s->v.s);

    s->type = V_TYPE_INT;
    s->v.i = val;

    return s;
}

/*!
  @brief Return numeric comparison of two scalars.
  @ingroup scalar_compare

  @param s1 Pointer to first scalar.
  @param s2 Pointer to second scalar.

  @retval -1 if \c s1 < \c s2
  @retval 0 if \c s1 = \c s2
  @retval 1 if \c s1 > \c s2

  This is designed for use in list sorting functions (vl_sort(),
  vl_insort(), etc.)
*/
int
vs_numcmp(vscalar **s1, vscalar **s2)
{
    double d1 = vs_dget(*s1);
    double d2 = vs_dget(*s2);
    return V_CMP(d1, d2);
}

/*!
  @brief Return pointer value of a scalar.
  @ingroup scalar_access

  @param s Scalar.

  @return Value.
*/
void *
vs_pget(vscalar *s)
{
    if (s == NULL)
	return NULL;

    VS_CHECK(s);

    if (s->type == V_TYPE_POINTER)
        return s->v.p;

    if (s->type == V_TYPE_UNDEF)
        return NULL;

    v_exception("vs_pget(): scalar is not a pointer");
    return NULL;
}

/* Print contents of a scalar */
void
vs_print(vscalar *s, FILE *fp)
{
    void *ptr;

    VS_CHECK(s);

    v_print_start();

    if        (s == NULL) {
        fprintf(fp, "INVALID\n");
    } else if (vs_type(s) == V_TYPE_UNDEF) {
        fprintf(fp, "undef\n");
    } else if (vs_type(s) == V_TYPE_POINTER) {
        ptr = vs_pget(s);
        if (v_type(ptr) == vscalar_type) {
            v_print_type(vscalar_type, ptr, fp);
            v_push_indent();
            v_indent(fp);
            v_print(ptr, fp);
            v_pop_indent();
        } else {
            v_print(ptr, fp);
        }
    } else {
        fprintf(fp, "%s\n", vs_sget_buf(s, buf));
    }

    v_print_finish();
}

/*!
  @brief Set a scalar to a pointer value.
  @ingroup scalar_modify

  @param s Scalar (or <tt>NULL</tt>).
  @param val Value to set.

  @return Scalar that was set.
*/
vscalar *
vs_pstore(vscalar *s, void *val)
{
    if (s == NULL)
	s = vs_create(V_TYPE_POINTER);

    VS_CHECK(s);

    if (s->type == V_TYPE_STRING)
	V_DEALLOC(s->v.s);

    s->type = V_TYPE_POINTER;
    s->v.p = val;

    return s;
}

/* Read scalar from a stream */
vscalar *
vs_read(FILE *fp)
{
    vscalar *s = NULL;
    double dval;
    char *sval;
    float fval;
    short type;
    int ival;

    /* Scalar type */
    if (!v_read_short(&type, fp))
        return NULL;

    /* Value */
    switch (type) {

    case V_TYPE_INT:
        if (v_read_long(&ival, fp))
            s = vs_icreate(ival);
        else
            return NULL;
        break;

    case V_TYPE_FLOAT:
        if (v_read_float(&fval, fp))
            s = vs_fcreate(fval);
        else
            return NULL;
        break;

    case V_TYPE_DOUBLE:
        if (v_read_double(&dval, fp))
            s = vs_dcreate(dval);
        else
            return NULL;
        break;

    case V_TYPE_STRING:
        if ((sval = v_read_string(fp)) != NULL)
            s = vs_screate(sval);
        else
            return NULL;
        break;

    case V_TYPE_POINTER:
        s = vs_pcreate(v_read(fp));
        break;

    case V_TYPE_UNDEF:
        s = vs_create(V_TYPE_UNDEF);
        break;

    default:
        v_exception("invalid scalar type");
        break;
    }

    return s;
}

/*!
  @brief   Create a scalar with a given type and value.
  @ingroup scalar_create
  @param   type Scalar type.
  @return  New scalar.

  The value must be of the required type, or chaos will surely result.
*/
vscalar *
vs_scalar(enum v_stype type, ...)
{
    vscalar *s = vs_create(type);
    va_list ap;
    char *str;

    va_start(ap, type);

    switch (type) {
    case V_TYPE_INT:
        s->v.i = va_arg(ap, int);
        break;
    case V_TYPE_FLOAT:
        s->v.f = va_arg(ap, double);
        break;
    case V_TYPE_DOUBLE:
        s->v.d = va_arg(ap, double);
        break;
    case V_TYPE_STRING:
        str = va_arg(ap, char *);
        s->v.s = (str == NULL ? NULL : V_STRDUP(str));
        break;
    case V_TYPE_POINTER:
        s->v.p = va_arg(ap, void *);
        break;
    }

    va_end(ap);
    return s;
}

/*!
  @brief Return string value of a scalar.
  @ingroup scalar_access

  @param s Scalar.

  @return Value.

  @note This is a pointer to an internal buffer, which will be clobbered
  by the next call.
*/
char *
vs_sget(vscalar *s)
{
    static char sval[BUFSIZ];
    return vs_sget_buf(s, sval);
}

/*!
  @brief Return string value of a scalar using a buffer.
  @ingroup scalar_access

  @param s Scalar.
  @param sval Buffer to copy into.

  @return Value.
*/
char *
vs_sget_buf(vscalar *s, char *sval)
{
    sval[0] = '\0';

    if (s == NULL)
	return sval;

    VS_CHECK(s);

    switch (s->type) {
    case V_TYPE_INT:
	sprintf(sval, "%d", s->v.i);
	break;
    case V_TYPE_FLOAT:
        sprintf(sval, FLOAT_FORMAT, s->v.f);
        break;
    case V_TYPE_DOUBLE:
	sprintf(sval, FLOAT_FORMAT, s->v.d);
	break;
    case V_TYPE_STRING:
        if (s->v.s != NULL)
            strcpy(sval, s->v.s);
	break;
    case V_TYPE_POINTER:
	sprintf(sval, "0x%p", s->v.p);
        break;
    default:
        /* Shuts compiler up */
        break;
    }

    return sval;
}

/*!
  @brief Return reference to the string value of a scalar.
  @ingroup scalar_access

  @param s Scalar.

  @return String value.
  @retval "" if not a string.

  An alternate way to access the string value of a scalar, this returns a
  reference to the stored string value (or the empty string, if the value
  is not a string). This differs from vs_sget() in that no conversion is
  done from other data types.
*/
char *
vs_sgetref(vscalar *s)
{
    if (s == NULL)
        return "";

    VS_CHECK(s);

    if (s->type == V_TYPE_STRING)
        return s->v.s;

    if (s->type != V_TYPE_UNDEF)
        v_exception("vs_sgetref(): scalar is not a string");

    return "";
}

/*!
  @brief Set a scalar to a string value.
  @ingroup scalar_modify

  @param s Scalar (or <tt>NULL</tt>).
  @param val Value to set.

  @return Scalar that was set.
*/
vscalar *
vs_sstore(vscalar *s, char *val)
{
    if (s == NULL)
	s = vs_create(V_TYPE_STRING);

    VS_CHECK(s);

    if (s->type == V_TYPE_STRING)
	V_DEALLOC(s->v.s);

    s->type = V_TYPE_STRING;
    s->v.s = (val == NULL ? NULL : V_STRDUP(val));

    return s;
}

/*!
  @brief Set a scalar to a string value based on a character array.
  @ingroup scalar_modify

  @param s Scalar (or <tt>NULL</tt>).
  @param base Array to set from.
  @param len Length of array.

  @return Scalar that was set.
*/
vscalar *
vs_sstore_len(vscalar *s, char *base, size_t len)
{
    int i;

    if (s == NULL)
	s = vs_create(V_TYPE_STRING);

    VS_CHECK(s);

    if (s->type == V_TYPE_STRING)
	V_DEALLOC(s->v.s);

    s->type = V_TYPE_STRING;
    s->v.s = V_ALLOC(char, len + 1);

    for (i = 0; i < len; i++)
        s->v.s[i] = base[i];

    s->v.s[len] = '\0';
    return s;
}

/* Thaw a scalar from file */
vscalar *
vs_thaw(FILE *fp)
{
    int token, brace = 0;
    vscalar *s = NULL;
    void *ptr;

    v_thaw_start();

    token = v_thaw_peek(fp);

    if (token == '{') {
        v_thaw_token(fp);
        token = v_thaw_peek(fp);
        brace = 1;
    }

    if (token == V_TOKEN_VTYPE || token == V_TOKEN_REF) {
        if ((ptr = v_thaw(fp)) != NULL) {
            s = vs_pcreate(ptr);
        } else {
            goto fail;
        }
    } else {
        switch (v_thaw_token(fp)) {
        case V_TOKEN_INT:
            s = vs_icreate(v_thaw_ivalue);
            break;
        case V_TOKEN_REAL:
            s = vs_dcreate(v_thaw_dvalue);
            break;
        case V_TOKEN_STRING:
            s = vs_screate(v_thaw_svalue);
            break;
        case V_TOKEN_NULL:
            s = vs_pcreate(NULL);
            break;
        case V_TOKEN_UNDEF:
            s = vs_create(V_TYPE_UNDEF);
            break;
        default:
            v_thaw_expected("scalar value");
            goto fail;
        }
    }

    if (brace && !v_thaw_follow(fp, '}', "close-brace"))
        goto fail;

    v_thaw_finish();
    return s;

  fail:
    v_thaw_finish();
    if (s != NULL)
        v_destroy(s);

    return NULL;
}

/* Traverse a scalar */
int
vs_traverse(vscalar *s, int (*func)())
{
    int val;

    VS_CHECK(s);

    if ((val = func(s)) != 0)
        return val;

    if (v_traverse_seen(s))
        return 0;

    v_push_traverse(s);

    if (vs_type(s) == V_TYPE_POINTER)
        val = v_traverse(vs_pget(s), func);

    v_pop_traverse();

    return val;
}

/*!
  @brief Return the type of a scalar.
  @ingroup scalar_access

  @param s Scalar.

  @return Its type.
*/
enum v_stype
vs_type(vscalar *s)
{
    if (s != NULL) {
        VS_CHECK(s);
        return s->type;
    }

    return V_TYPE_NULL;
}

/*!
  @brief Undefine a scalar.
  @ingroup scalar_modify

  @param s Scalar.
*/
void
vs_undef(vscalar *s)
{
    VS_CHECK(s);

    if (s->type == V_TYPE_STRING)
	V_DEALLOC(s->v.s);

    s->type = V_TYPE_UNDEF;
}

/* Write scalar to a stream */
int
vs_write(vscalar *s, FILE *fp)
{
    VS_CHECK(s);

    /* Scalar type */
    if (!v_write_short(s->type, fp))
        return 0;

    /* Value */
    switch (s->type) {
    case V_TYPE_INT:
        if (!v_write_long(s->v.i, fp))
            return 0;
        break;
    case V_TYPE_FLOAT:
        if (!v_write_float(s->v.f, fp))
            return 0;
        break;
    case V_TYPE_DOUBLE:
        if (!v_write_double(s->v.d, fp))
            return 0;
        break;
    case V_TYPE_STRING:
        if (!v_write_string(s->v.s, fp))
            return 0;
        break;
    case V_TYPE_POINTER:
        if (!v_write(s->v.p, fp))
            return 0;
        break;
    }

    return 1;
}

/* Dump XML scalar to file */
int
vs_xmldump(vscalar *s, FILE *fp)
{
    void *ptr;

    VS_CHECK(s);

    v_xmldump_start(fp);

    switch(vs_type(s)) {

    case V_TYPE_INT:
        sprintf(buf, "%d", vs_iget(s));
        v_xmldump_tag(fp, "int", "value", buf, NULL);
        break;

    case V_TYPE_FLOAT:
        sprintf(buf, FLOAT_FORMAT, vs_fget(s));
        v_xmldump_tag(fp, "float", "value", buf, NULL);
        break;

    case V_TYPE_DOUBLE:
        sprintf(buf, FLOAT_FORMAT, vs_dget(s));
        v_xmldump_tag(fp, "double", "value", buf, NULL);
        break;

    case V_TYPE_STRING:
        v_xmldump_data(fp, "string", vs_sget_buf(s, buf));
        break;

    case V_TYPE_POINTER:
        ptr = vs_pget(s);
        if (!v_xmldump(ptr, fp))
            return 0;

        break;

    case V_TYPE_UNDEF:
        v_xmldump_tag(fp, "undef", NULL);
        break;
    }

    v_xmldump_finish(fp);

    return 1;
}

/* Dump YAML scalar to file */
int
vs_yamldump(vscalar *s, FILE *fp)
{
    VS_CHECK(s);

    if (!v_yaml_start(fp))
        return 0;

    if (!v_yaml_write_scalar(s, fp))
        return 0;

    if (!v_yaml_finish(fp))
        return 0;

    return 1;
}
