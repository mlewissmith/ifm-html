/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup yaml YAML functions
  @ingroup methods

  These functions read and write Vars objects in YAML format.
*/

/*!
  @defgroup yaml_high High-level functions
  @ingroup yaml

  These functions are for high-level use.
*/

/*!
  @defgroup yaml_low Low-level functions
  @ingroup yaml

  These functions are for use when defining new object types (see @ref
  extend).
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_LIBYAML
#include <yaml.h>
#endif

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-hash.h"
#include "vars-system.h"
#include "vars-type.h"
#include "vars-yaml.h"

#define FLOAT_FORMAT "%.12g"

/* Output width */
#define WIDTH 65

/* Current YAML filename */
static char *filename = NULL;

#ifdef HAVE_LIBYAML
/* Current YAML emitter */
static yaml_emitter_t emitter;
#endif

/* YAML writing depth */
static int depth = 0;

/* YAML error buffer */
V_NBUF_DECL(ebuf);

/* Internal functions */
#ifdef HAVE_LIBYAML
static void *v_yaml_read_object(vlist *events);
static int v_yaml_next_event(vlist *events);
static void v_yaml_read_err(char *msg, yaml_parser_t *parser);
static char *v_yaml_tag(char *tag);
#endif

/*!
  @brief   Read YAML objects from a stream.
  @ingroup yaml_high
  @param   fp Stream to read.
  @return  Pointer to Vars object.
  @retval  NULL if it failed.
*/
void *
v_yaml_read(FILE *fp)
{
#ifdef HAVE_LIBYAML
    vlist *events = NULL, *objects = NULL;
    yaml_parser_t parser;
    void *object = NULL;
    yaml_event_t event;
    int loop = 1;

    /* Initalize */
    if (yaml_parser_initialize(&parser)) {
        yaml_parser_set_input_file(&parser, fp);
    } else {
        v_yaml_read_err("can't initialize parser", NULL);
        return NULL;
    }

    /* Parse input stream into events */
    events = vl_create();

    while (loop) {
        if (yaml_parser_parse(&parser, &event)) {
            switch (event.type) {

            case YAML_SEQUENCE_START_EVENT:
            case YAML_SEQUENCE_END_EVENT:
            case YAML_MAPPING_START_EVENT:
            case YAML_MAPPING_END_EVENT:
                vl_ipush(events, event.type);
                break;

            case YAML_SCALAR_EVENT:
                vl_spush(events, event.data.scalar.value);
                break;

            case YAML_STREAM_END_EVENT:
                loop = 0;
                break;
            }

            yaml_event_delete(&event);
        } else {
            v_yaml_read_err(NULL, &parser);
            goto cleanup;
        }
    }

    /* Scan event list and build data structure */
    objects = vl_create();
    while ((object = v_yaml_read_object(events)) != NULL)
        vl_ppush(objects, object);

  cleanup:
    /* Clean up */
    yaml_parser_delete(&parser);

    if (events != NULL)
        vl_destroy(events);

    /* If more than one document, return list of them */
    if (objects != NULL && vl_length(objects) > 1)
        return objects;

    /* Otherwise, extract the document and return it */
    if (objects != NULL) {
        object = vl_phead(objects);
        vl_destroy(objects);
    }

    return object;
#else
    v_unavailable("v_yaml_read()");
#endif
}

#ifdef HAVE_LIBYAML
/* Read an object from a parsed YAML event list */
static void *
v_yaml_read_object(vlist *events)
{
    void *object;
    vlist *list;
    vhash *hash;
    char *key;

    if (vl_length(events) == 0)
        return NULL;

    switch (v_yaml_next_event(events)) {

    case YAML_SEQUENCE_START_EVENT:
        vl_ishift(events);
        list = vl_create();

        while (v_yaml_next_event(events) != YAML_SEQUENCE_END_EVENT) {
            object = v_yaml_read_object(events);

            if (vl_check(object) || vh_check(object))
                vl_ppush(list, object);
            else
                vl_spush(list, object);
        }

        vl_ishift(events);
        return list;

    case YAML_MAPPING_START_EVENT:
        vl_ishift(events);
        hash = vh_create();

        while (v_yaml_next_event(events) != YAML_MAPPING_END_EVENT) {
            key = v_yaml_read_object(events);
            object = v_yaml_read_object(events);

            if (vl_check(object) || vh_check(object))
                vh_pstore(hash, key, object);
            else
                vh_sstore(hash, key, object);
        }

        vl_ishift(events);
        return hash;

    case YAML_SCALAR_EVENT:
        return vl_sshift(events);
    }

    v_exception("unhandled yaml event: %d\n", vl_ihead(events));
    return NULL;
}

/* Return next event from a YAML event list */
static int
v_yaml_next_event(vlist *events)
{
    vscalar *s = vl_head(events);

    if (vs_type(s) == V_TYPE_INT)
        return vs_iget(s);
    else
        return YAML_SCALAR_EVENT;
}
#endif

/*!
  @brief   Read YAML objects from a file.
  @ingroup yaml_high
  @param   file File to read.
  @return  Pointer to Vars object.
  @retval  NULL if it failed.
*/
void *
v_yaml_read_file(char *file)
{
#ifdef HAVE_LIBYAML
    void *ptr = NULL;
    V_BUF_DECL;
    FILE *fp;

    if ((fp = v_open(file, "r")) != NULL) {
        filename = file;
        ptr = v_yaml_read(fp);
        filename = NULL;
        v_close(fp);
    } else {
        V_BUF_SET2("can't open %s: %s", file, strerror(errno));
        v_yaml_read_err(V_BUF_VAL, NULL);
    }

    return ptr;
#else
    v_unavailable("v_yaml_read_file()");    
#endif
}

#ifdef HAVE_LIBYAML
/* Give a YAML read error */
static void
v_yaml_read_err(char *msg, yaml_parser_t *parser)
{
    V_BUF_DECL;

    V_NBUF_SET(ebuf, filename ? filename : "<stdin>");

    if (parser != NULL)
        V_NBUF_ADD2(ebuf, ", line %d, column %d",
                    parser->problem_mark.line,
                    parser->problem_mark.column);

    if (msg != NULL)
        V_NBUF_ADD1(ebuf, ": %s", msg);

    if (parser != NULL)
        V_NBUF_ADD1(ebuf, ": %s", parser->problem);

    v_exception("%s", V_NBUF_VAL(ebuf));
}
#endif

/*!
  @brief   Write contents of a pointer in YAML format.
  @ingroup yaml_high
  @param   ptr Pointer to Vars object.
  @param   fp Stream to write.
  @return  Whether successful.
  @todo    Deal with multiply-referenced pointers, using anchors.
*/
int
v_yaml_write(void *ptr, FILE *fp)
{
#ifdef HAVE_LIBYAML
    vtype *t;
    int ok;

    if (!v_yaml_start(fp))
        return 0;

    if (ptr != NULL) {
        /* Get pointer type */
        if ((t = v_type(ptr)) == NULL)
            v_fatal("v_yaml_write(): unknown pointer type");

        /* Get dump function */
        if (t->yamldump == NULL)
            v_fatal("v_yaml_write(): type %s has no yamldump function", t->name);

        /* Call it */
        if (!t->yamldump(ptr, fp))
            return 0;
    }

    if (!v_yaml_finish(fp))
        return 0;

    return 1;
#else
    v_unavailable("v_yaml_write()");
#endif
}

/*!
  @brief   Write contents of a pointer to a file in YAML format.
  @ingroup yaml_high
  @param   ptr Pointer to Vars object.
  @param   file File to write.
  @retval  Whether successful.
  @todo    Set error buffer on failure.
*/
int
v_yaml_write_file(void *ptr, char *file)
{
#ifdef HAVE_LIBYAML
    int ok = 0;
    V_BUF_DECL;
    FILE *fp;

    if ((fp = v_open(file, "w")) != NULL) {
        filename = file;
        ok = v_yaml_write(ptr, fp);
        filename = NULL;
        v_close(fp);
    } else {
        V_BUF_SET2("can't open %s: %s", file, strerror(errno));
    }

    return ok;
#else
    v_unavailable("v_yaml_write_file()");
#endif
}

/* Write a scalar to YAML stream */
int
v_yaml_write_scalar(vscalar *s, FILE *fp)
{
    int ok;

    switch (vs_type(s)) {

    case V_TYPE_POINTER:
        ok = v_yaml_write(vs_pget(s), fp);
        break;

    default:
        ok = v_yaml_write_string(vs_sget(s), fp);
        break;
    }

    return ok;
}

/* Write a list to YAML stream */
int
v_yaml_write_list(vlist *l, FILE *fp)
{
    vscalar *s;
    viter i;

    if (!v_yaml_start_list(fp, NULL))
        return 0;

    v_iterate(l, i) {
        s = vl_iter_val(i);
        if (!v_yaml_write_scalar(s, fp))
            return 0;
    }

    if (!v_yaml_finish_list(fp))
        return 0;

    return 1;
}

/* Write a hash to YAML stream */
int
v_yaml_write_hash(vhash *h, FILE *fp)
{
    vscalar *s;
    char *key;
    viter i;

    if (!v_yaml_start_hash(fp, NULL))
        return 0;

    v_iterate(h, i) {
        key = vh_iter_key(i);
        if (!v_yaml_write_string(key, fp))
            return 0;

        s = vh_iter_val(i);
        if (!v_yaml_write_scalar(s, fp))
            return 0;
    }

    if (!v_yaml_finish_hash(fp))
        return 0;

    return 1;
}

/* Write a double to YAML stream */
int
v_yaml_write_double(double val, FILE *fp)
{
    V_BUF_DECL;
    V_BUF_SET1(FLOAT_FORMAT, val);
    return v_yaml_write_string(V_BUF_VAL, fp);
}

/* Write an integer to YAML stream */
int
v_yaml_write_int(int val, FILE *fp)
{
    V_BUF_DECL;
    V_BUF_SET1("%d", val);
    return v_yaml_write_string(V_BUF_VAL, fp);
}

/* Write a string to YAML stream */
int
v_yaml_write_string(char *val, FILE *fp)
{
#ifdef HAVE_LIBYAML
    yaml_event_t event;

    if (!yaml_scalar_event_initialize(&event, NULL, NULL, val, strlen(val), 
                                      1, 1, YAML_ANY_SCALAR_STYLE))
        return 0;

    if (!yaml_emitter_emit(&emitter, &event))
        return 0;

    return 1;
#else
    v_unavailable("v_yaml_string()");
#endif
}

/* Start writing YAML */
int
v_yaml_start(FILE *fp)
{
#ifdef HAVE_LIBYAML
    yaml_event_t event;

    if (depth++ == 0) {
        /* Initialize emitter */
        if (yaml_emitter_initialize(&emitter)) {
            yaml_emitter_set_output_file(&emitter, fp);
            yaml_emitter_set_width(&emitter, WIDTH);
        } else {
            depth = 0;
            return 0;
        }

        /* Write header */
        fprintf(fp, "%YAML 1.1\n");
        fprintf(fp, "---\n");

        /* Send initial events */
        if (!yaml_stream_start_event_initialize(&event, YAML_ANY_ENCODING))
            goto fail;

        if (!yaml_emitter_emit(&emitter, &event))
            goto fail;

        if (!yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 1))
            goto fail;

        if (!yaml_emitter_emit(&emitter, &event))
            goto fail;
    }

    return 1;

  fail:
    yaml_emitter_delete(&emitter);
    return 0;
#else
    v_unavailable("v_yaml_start()");
#endif
}

/* Finish writing YAML */
int
v_yaml_finish(FILE *fp)
{
#ifdef HAVE_LIBYAML
    yaml_event_t event;

    if (--depth == 0) {
        /* Send final events */
        if (!yaml_document_end_event_initialize(&event, 1))
            goto fail;

        if (!yaml_emitter_emit(&emitter, &event))
            goto fail;

        if (!yaml_stream_end_event_initialize(&event))
            goto fail;

        if (!yaml_emitter_emit(&emitter, &event))
            goto fail;

        /* Clean up */
        yaml_emitter_delete(&emitter);
    }

    return 1;

  fail:
    yaml_emitter_delete(&emitter);
    return 0;
#else
    v_unavailable("v_yaml_finish()");
#endif
}

/* Write a list start marker to YAML stream */
int
v_yaml_start_list(FILE *fp, char *tag)
{
#ifdef HAVE_LIBYAML
    yaml_event_t event;

    tag = v_yaml_tag(tag);
    if (!yaml_sequence_start_event_initialize(&event, NULL, tag, tag == NULL,
                                              YAML_ANY_SEQUENCE_STYLE))
        return 0;

    if (!yaml_emitter_emit(&emitter, &event))
        return 0;

    return 1;
#else
    v_unavailable("v_yaml_start_list()");
#endif
}

/* Write a list end marker to YAML stream */
int
v_yaml_finish_list(FILE *fp)
{
#ifdef HAVE_LIBYAML
    yaml_event_t event;

    if (!yaml_sequence_end_event_initialize(&event))
        return 0;

    if (!yaml_emitter_emit(&emitter, &event))
        return 0;

    return 1;
#else
    v_unavailable("v_yaml_finish_list()");
#endif
}

/* Write a hash start marker to YAML stream */
int
v_yaml_start_hash(FILE *fp, char *tag)
{
#ifdef HAVE_LIBYAML
    yaml_event_t event;

    tag = v_yaml_tag(tag);
    if (!yaml_mapping_start_event_initialize(&event, NULL, tag, tag == NULL,
                                             YAML_ANY_MAPPING_STYLE))
        return 0;

    if (!yaml_emitter_emit(&emitter, &event))
        return 0;

    return 1;
#else
    v_unavailable("v_yaml_start_hash()");
#endif
}

/* Write a hash end marker to YAML stream */
int
v_yaml_finish_hash(FILE *fp)
{
#ifdef HAVE_LIBYAML
    yaml_event_t event;

    if (!yaml_mapping_end_event_initialize(&event))
        return 0;

    if (!yaml_emitter_emit(&emitter, &event))
        return 0;

    return 1;
#else
    v_unavailable("v_yaml_finish_hash()");
#endif
}

/*!
  @brief   Return last YAML parser error.
  @ingroup yaml_high
  @return  Error string.
  @retval  NULL if no errors found.
*/
char *
v_yaml_error(void)
{
    if (ebuf != NULL)
        return vb_get(ebuf);

    return NULL;
}

#ifdef HAVE_LIBYAML
/* Return a Vars yaml tag */
static char *
v_yaml_tag(char *tag)
{
    static char buf[30];

    if (tag != NULL) {
        sprintf(buf, "vars/%s", tag);
        tag = buf;
    }

    return tag;
}
#endif
