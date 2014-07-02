/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup freeze Freeze/thaw functions
  @ingroup methods

  These functions perform text I/O on objects.  See @ref persist for more
  details.
*/

/*!
  @defgroup freeze_high High-level functions
  @ingroup freeze

  These functions are for high-level use.
*/

/*!
  @defgroup freeze_low Low-level functions
  @ingroup freeze

  These functions are for use when defining new object types (see @ref
  extend).
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-freeze.h"
#include "vars-freeze-lex.h"
#include "vars-hash.h"
#include "vars-macros.h"
#include "vars-system.h"
#include "vars-utils.h"

#define IDMAP_INIT \
        if (idmap == NULL) idmap = vh_create_size(1000)

/* Current token values */
int v_thaw_ivalue = 0;
double v_thaw_dvalue = 0.0;
char *v_thaw_svalue = "";
void *v_thaw_pvalue = NULL;

/* Current thaw filename */
static char *filename = NULL;

/* Freeze tag count */
static int ft_tag = 0;

/* Freeze/thaw recursion depth */
static int ft_depth = 0;

/* Multiple reference flags */
static vhash *multiref = NULL;

/* Mapping of ID to data */
static vhash *idmap = NULL;

/* Lookahead token */
static enum v_tokentype lookahead = 0;

/* No. of thaw errors */
static int num_errors = 0;

V_NBUF_DECL(ebuf);

/* Internal functions */
static int flag_refs(void *ptr);

/* Flag multiple references to pointers */
static int
flag_refs(void *ptr)
{
    char *key, kbuf[V_HEXSTRING_SIZE];

    if (v_traverse_seen(ptr)) {
        key = vh_pkey_buf(ptr, kbuf);
        vh_undef(multiref, key);
    }

    return 0;
}

/*!
  @brief   Freeze contents of a pointer.
  @ingroup freeze_high
  @param   ptr Pointer to Vars object.
  @param   fp Stream to write.
  @return  Whether successful.
*/
int
v_freeze(void *ptr, FILE *fp)
{
    char *key, kbuf[V_HEXSTRING_SIZE];
    static int depth = 0;
    int id, code = 1;
    vtype *t;

    IDMAP_INIT;

    /* Check special case */
    if (ptr == NULL) {
        v_freeze_start(fp);
        fprintf(fp, "%s", V_TOKEN_NULL_STRING);
        v_freeze_finish(fp);
        return code;
    }

    /* Get pointer type */
    if ((t = v_type(ptr)) == NULL)
        v_fatal("v_freeze(): unknown pointer type");

    if (t->freeze == NULL)
        v_fatal("v_freeze(): type %s has no freeze function", t->name);

    /* Initialise if required */
    v_freeze_start(fp);

    if (depth++ == 0) {
        vh_init(multiref);
        v_traverse(ptr, flag_refs);
    }        

    key = vh_pkey_buf(ptr, kbuf);
    id = vh_iget(idmap, key);

    if (id == 0) {
        /* Flag it as written */
        if (vh_exists(multiref, key)) {
            id = ++ft_tag;
            vh_istore(idmap, key, id);
        }

        /* Write name */
        fprintf(fp, "%s ", t->name);

        /* Write ID if required */
        if (id)
            fprintf(fp, "%d ", id);

        /* Write the data */
        if (!t->freeze(ptr, fp))
            code = 0;
    } else {
        /* Seen before -- write old ID */
        fprintf(fp, "%s %d", V_TOKEN_REF_STRING, id);
    }

    /* Clean up if required */
    key = vh_pkey_buf(ptr, kbuf);

    if (--depth == 0)
        vh_empty(multiref);

    v_freeze_finish(fp);

    return code;
}

/*!
  @brief   Freeze a pointer to a file.
  @ingroup freeze_high
  @param   ptr Pointer to Vars object.
  @param   file File to write.
  @retval  Whether successful.
*/
int
v_freeze_file(void *ptr, char *file)
{
    int ok = 0;
    FILE *fp;

    if ((fp = fopen(file, "w")) != NULL) {
        ok = v_freeze(ptr, fp);
        fclose(fp);
    }

    if (!ok)
        v_exception("can't freeze file '%s'", file);

    return ok;
}

/*!
  @brief   Finish freezing.
  @ingroup freeze_low
  @param   fp Stream.
*/
void
v_freeze_finish(FILE *fp)
{
    if (--ft_depth == 0) {
        vh_empty(idmap);
        fputc('\n', fp);
    }
}

/*!
  @brief   Start freezing.
  @ingroup freeze_low
  @param   fp Stream.
*/
void
v_freeze_start(FILE *fp)
{
    if (ft_depth++ == 0)
        ft_tag = 0;
}

/*!
  @brief   Freeze a string to file.
  @ingroup freeze_low
  @param   string String to freeze.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_freeze_string(char *string, FILE *fp)
{
    fputc('"', fp);

    while (*string != '\0') {
        switch (*string) {
        case '\n':
            fputc('\\', fp);
            fputc('n', fp);
            break;
        case '\\':
            fputc('\\', fp);
            fputc('\\', fp);
            break;
        case '"':
            fputc('\\', fp);
            fputc('"', fp);
            break;
        default:
            fputc(*string, fp);
            break;
        }

        string++;
    }

    fputc('"', fp);
    return 1;
}

/*!
  @brief   Thaw a pointer from file.
  @ingroup freeze_high
  @param   fp Stream to read.
  @return  Pointer to Vars object.
  @retval  NULL if it failed.
  @see     v_thaw_error()
*/
void *
v_thaw(FILE *fp)
{
    char idbuf[V_HEXSTRING_SIZE];
    vtype *type;
    void *data;
    int id;

    v_thaw_start();

    /* Get data type */
    switch (v_thaw_token(fp)) {

    case V_TOKEN_VTYPE:
        type = v_thaw_pvalue;

        /* Check data type can be thawed */
        if (type->thaw == NULL) {
            v_thaw_err("type '%s' has no thaw function", type->name);
            goto fail;
        }

        /* Read data ID (if any) */
        if (v_thaw_peek(fp) == V_TOKEN_INT) {
            v_thaw_token(fp);
            id = v_thaw_ivalue;
        } else {
            id = 0;
        }

        /* Thaw the data */
        data = type->thaw(fp);

        /* Add to ID mapping if required */
        if (id) {
            sprintf(idbuf, "%d", id);
            if (!vh_exists(idmap, idbuf)) {
                vh_pstore(idmap, idbuf, data);
            } else {
                v_thaw_err("reference tag %d already defined", id);
                goto fail;
            }
        }

        break;

    case V_TOKEN_REF:
        /* Read data ID */
        if (v_thaw_token(fp) == V_TOKEN_INT) {
            id = v_thaw_ivalue;
        } else {
            v_thaw_expected("reference tag");
            goto fail;
        }

        /* Look up referenced data */
        sprintf(idbuf, "%d", id);
        if ((data = vh_pget(idmap, idbuf)) == NULL) {
            v_thaw_err("reference tag %d not yet defined", id);
            goto fail;
        }

        break;

    default:
        v_thaw_expected("type name or reference");
        goto fail;
    }

    /* Clean up */
    v_thaw_finish();
    return data;

  fail:
    v_thaw_finish();
    return NULL;
}

/*!
  @brief   Thaw a double value.
  @ingroup freeze_low
  @param   fp Stream.
  @param[out]   value Pointer to value.
  @return  Whether successful.
*/
int
v_thaw_double(FILE *fp, double *value)
{
    switch (v_thaw_token(fp)) {
    case V_TOKEN_INT:
        *value = (double) v_thaw_ivalue;
        break;
    case V_TOKEN_REAL:
        *value = v_thaw_dvalue;
        break;
    default:
        v_thaw_expected("integer or real number");
        return 0;
    }

    return 1;
}

/*!
  @brief   Give a thaw error.
  @ingroup freeze_low
  @param   fmt Format string.
*/
void
v_thaw_err(char *fmt, ...)
{
    V_BUF_DECL;
    char *msg;

    num_errors++;

    V_BUF_FMT(fmt, msg);

    if (filename != NULL && line_number > 0)
        V_NBUF_SET2(ebuf, "%s, line %d: ", filename, line_number);
    else if (line_number > 0)
        V_NBUF_SET1(ebuf, "line %d: ", line_number);
    else if (filename != NULL)
        V_NBUF_SET1(ebuf, "%s: ", filename);
    else
        V_NBUF_INIT(ebuf);

    V_NBUF_ADD(ebuf, msg);
    v_exception("%s", V_NBUF_VAL(ebuf));
}

/*!
  @brief   Return the last thaw error.
  @ingroup freeze_high
  @return  Error string.
  @retval  NULL if no errors found.
*/
char *
v_thaw_error(void)
{
    if (ebuf != NULL)
        return vb_get(ebuf);

    return NULL;
}

/*!
  @brief   Give expected-token error.
  @ingroup freeze_low
  @param   desc Description of what was expected.
*/
void
v_thaw_expected(char *desc)
{
    char *text = thaw_text();

    if (strlen(text) == 0)
        v_thaw_err("expected %s (last token: EOF)", desc);
    else if (text[0] == '"')
        v_thaw_err("expected %s (last token: '%c')", desc, text[0]);
    else
        v_thaw_err("expected %s (last token: '%s')", desc, text);
}

/*!
  @brief   Thaw a pointer from a file.
  @ingroup freeze_high
  @param   file File to read.
  @return  Pointer to Vars object.
  @retval  NULL if it failed.
*/
void *
v_thaw_file(char *file)
{
    void *ptr = NULL;
    FILE *fp;

    if ((fp = fopen(file, "r")) != NULL) {
        filename = file;
        ptr = v_thaw(fp);
        filename = NULL;
        fclose(fp);
    } else {
        v_thaw_err("can't open %s: %s", file, strerror(errno));
    }

    return ptr;
}

/*!
  @brief   Set thaw filename.
  @ingroup freeze_low
  @param   file Filename.
*/
void
v_thaw_filename(char *file)
{
    filename = file;
}

/*!
  @brief   Finish thawing.
  @ingroup freeze_low
*/
void
v_thaw_finish(void)
{
    if (--ft_depth == 0) {
        vh_empty(idmap);
        filename = NULL;
        thaw_restart();
    }
}

/*!
  @brief   Expect a particular token to follow.
  @ingroup freeze_low
  @param   fp Stream.
  @param   token Token expected.
  @param   desc Description of what's expected.
  @return  Whether it followed.
*/
int
v_thaw_follow(FILE *fp, enum v_tokentype token, char *desc)
{
    if (v_thaw_token(fp) == token)
        return 1;

    v_thaw_expected(desc);
    return 0;
}

/*!
  @brief   Get line number of last thawed line.
  @ingroup freeze_low
  @return  Line number.
*/
int
v_thaw_getline(void)
{
    return last_line_number;
}

/*!
  @brief   Thaw an int value.
  @ingroup freeze_low
  @param   fp Stream.
  @param[out]   value Pointer to value.
  @return  Whether successful.
*/
int
v_thaw_int(FILE *fp, int *value)
{
    if (v_thaw_token(fp) == V_TOKEN_INT) {
        *value = v_thaw_ivalue;
        return 1;
    }

    v_thaw_expected("integer");
    return 0;
}

/*!
  @brief   Look at the next token without reading it.
  @ingroup freeze_low
  @param   fp Stream.
  @return  Next token.
*/
enum v_tokentype
v_thaw_peek(FILE *fp)
{
    if (!lookahead)
        lookahead = v_thaw_token(fp);

    return lookahead;
}

/*!
  @brief   Set current line number.
  @ingroup freeze_low
  @param   num Line number.
*/
void
v_thaw_setline(int num)
{
    start_line = num;
}

/*!
  @brief   Start thawing.
  @ingroup freeze_low
*/
void
v_thaw_start(void)
{
    static int decl = 0;

    /* Declare standard types if required */
    if (!decl) {
        v_declare();
        decl++;
    }

    /* Initialise if required */
    if (ft_depth++ == 0) {
        line_number = start_line;
        num_errors = 0;
        lookahead = 0;
    }

    IDMAP_INIT;
}

/*!
  @brief   Thaw a string.
  @ingroup freeze_low
  @param   fp Stream.
  @param[out]   value Pointer to value.
  @return  Whether successful.
*/
int
v_thaw_string(FILE *fp, char **value)
{
    if (v_thaw_token(fp) == V_TOKEN_STRING) {
        *value = v_thaw_svalue;
        return 1;
    }

    v_thaw_expected("string");
    return 0;
}

/*!
  @brief   Get the next token.
  @ingroup freeze_low
  @param   fp Stream.
  @return  Next token.
*/
enum v_tokentype
v_thaw_token(FILE *fp)
{
    enum v_tokentype token;

    if (lookahead) {
        token = lookahead;
        lookahead = 0;
    } else {
        token = thaw_token(fp);

        if (V_DEBUG(V_DBG_INTERN)) {
            fprintf(stderr, "Read token: ");

            switch (token) {
            case V_TOKEN_INT:
                fprintf(stderr, "integer (%d)", v_thaw_ivalue);
                break;
            case V_TOKEN_REAL:
                fprintf(stderr, "real (%g)", v_thaw_dvalue);
                break;
            case V_TOKEN_STRING:
                fprintf(stderr, "string (%s)", v_thaw_svalue);
                break;
            case V_TOKEN_VTYPE:
                fprintf(stderr, "object type (%s)", v_thaw_svalue);
                break;
            case V_TOKEN_REF:
                fprintf(stderr, "object reference");
                break;
            case V_TOKEN_NULL:
                fprintf(stderr, "NULL");
                break;
            case V_TOKEN_UNDEF:
                fprintf(stderr, "UNDEF");
                break;
            case V_TOKEN_ID:
                fprintf(stderr, "ID name (%s)", v_thaw_svalue);
                break;
            case V_TOKEN_ERROR:
                fprintf(stderr, "error");
                break;
            default:
                fprintf(stderr, "literal (%s)", thaw_text());
                break;
            }

            fprintf(stderr, "\n");
        }
    }

    return token;
}
