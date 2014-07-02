/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup xml XML functions
  @ingroup methods

  These functions dump Vars objects in XML format.

  @warning XML output is currently only partially implemented, and there's
  no documentation on the XML format.
*/

/*!
  @defgroup xml_high High-level functions
  @ingroup xml

  These functions are for high-level use.
*/

/*!
  @defgroup xml_low Low-level functions
  @ingroup xml

  These functions are for use when defining new object types (see @ref
  extend).
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "vars-config.h"
#include "vars-hash.h"
#include "vars-type.h"
#include "vars-xml.h"

#define XMLHEADER "<?xml version=\"1.0\"?>"

static int xml_tag = 0;         /* Tag count */
static int xml_depth = 0;       /* Recursion depth */
static vhash *multiref = NULL;  /* Multiple reference flags */
static vhash *seen = NULL;      /* Data already processed */

/* Internal functions */
static int flag_refs(void *ptr);
static void v_xmldump_writetag(FILE *fp, char *tag, va_list ap);

/* Flag multiple references to pointers */
static int
flag_refs(void *ptr)
{
    char *key, kbuf[20];

    if (v_traverse_seen(ptr)) {
        key = vh_pkey_buf(ptr, kbuf);
        vh_undef(multiref, key);
    }

    return 0;
}

/*!
  @brief   Dump contents of a pointer in XML format.
  @ingroup xml_high
  @param   ptr Pointer to Vars structure.
  @param   fp Stream to write.
  @return  Whether successful.
*/
int
v_xmldump(void *ptr, FILE *fp)
{
    char *key, kbuf[20], nbuf[20], idbuf[20], *cp;
    static vhash *stack = NULL;
    static int depth = 0;
    int id, code = 1;
    vtype *t;

    if (seen == NULL)
        seen = vh_create_size(1000);

    if (stack == NULL)
        stack = vh_create();

    v_xmldump_start(fp);
    v_pop_indent();

    if (ptr != NULL) {
        /* Get pointer type */
        if ((t = v_type(ptr)) == NULL)
            v_fatal("v_xmldump(): unknown pointer type");

        if (t->xmldump == NULL)
            v_fatal("v_xmldump(): type %s has no xmldump function", t->name);

        /* Initialise if required */
        if (depth++ == 0) {
            vh_init(multiref);
            v_traverse(ptr, flag_refs);
        }        

        key = vh_pkey_buf(ptr, kbuf);
        id = vh_iget(seen, key);

        if (id == 0) {
            /* Flag it as written */
            if (vh_exists(multiref, key)) {
                id = ++xml_tag;
                vh_istore(seen, key, id);
            }

            vh_istore(stack, key, 1);

            /* Write name (and ID if required) */
            strcpy(nbuf, t->name);
            for (cp = nbuf; *cp != '\0'; cp++)
                *cp = tolower(*cp);

            if (id) {
                sprintf(idbuf, "%d", id);
                v_xmldump_tag_start(fp, nbuf, "id", idbuf, NULL);
            } else {
                v_xmldump_tag_start(fp, nbuf, NULL);
            }

            /* Write the data */
            if (t->xmldump(ptr, fp))
                v_xmldump_tag_finish(fp, nbuf);
            else
                code = 0;
        } else {
            /* Seen before -- write old ID */
            sprintf(idbuf, "%d", id);
            v_xmldump_tag(fp, "ref", "id", idbuf, NULL);

            /* Give error if invalid reference */
            if (vh_iget(stack, key))
                code = 0;
        }

        /* Clean up if required */
        key = vh_pkey_buf(ptr, kbuf);
        vh_delete(stack, key);

        if (--depth == 0)
            vh_empty(multiref);
    }

    v_push_indent();
    v_xmldump_finish(fp);

    return code;
}

/*!
  @brief   Write XML matched tag and associated data.
  @ingroup xml_low
  @param   fp Stream.
  @param   tag Tag.
  @param   data String data.
*/
void
v_xmldump_data(FILE *fp, char *tag, char *data)
{
    v_indent(fp);
    fprintf(fp, "<%s>", tag);
    v_xmldump_string(fp, data);
    fprintf(fp, "</%s>\n", tag);
}

/*!
  @brief   Dump contents of a pointer in XML format to a file.
  @ingroup xml_high
  @param   ptr Pointer to Vars object.
  @param   file File to write.
  @return  Whether successful.
*/
int
v_xmldump_file(void *ptr, char *file)
{
    int ok = 0;
    FILE *fp;

    if ((fp = fopen(file, "wb")) != NULL) {
        ok = v_xmldump(ptr, fp);
        fclose(fp);
    }

    return ok;
}

/*!
  @brief   Start dumping XML.
  @ingroup xml_low
  @param   fp Stream.
*/
void
v_xmldump_start(FILE *fp)
{
    v_push_indent();

    if (xml_depth++ == 0) {
        xml_tag = 0;
        fprintf(fp, "%s\n\n", XMLHEADER);
    }
}

/*!
  @brief   Finish dumping XML.
  @ingroup xml_low
  @param   fp Stream.
*/
void
v_xmldump_finish(FILE *fp)
{
    v_pop_indent();
    if (--xml_depth == 0)
        vh_empty(seen);
}

/*!
  @brief   Dump an XML string to file.
  @ingroup xml_low
  @param   fp Stream.
  @param   string String to dump.
*/
void
v_xmldump_string(FILE *fp, char *string)
{
    while (*string != '\0') {
        switch (*string) {
        case '<':
            fputs("&lt;", fp);
            break;
        case '>':
            fputs("&gt;", fp);
            break;
        case '&':
            fputs("&amp;", fp);
            break;
        case '"':
            fputs("&quot;", fp);
            break;
        default:
            fputc(*string, fp);
            break;
        }

        string++;
    }
}

/*!
  @brief   Write complete XML tag.
  @ingroup xml_low
  @param   fp Stream.
  @param   tag Tag.
*/
void
v_xmldump_tag(FILE *fp, char *tag, ...)
{
    va_list ap;
    va_start(ap, tag);
    v_xmldump_writetag(fp, tag, ap);
    fprintf(fp, " />\n");
}

/*!
  @brief   Write start XML tag.
  @ingroup xml_low
  @param   fp Stream.
  @param   tag Tag.
*/
void
v_xmldump_tag_start(FILE *fp, char *tag, ...)
{
    va_list ap;
    va_start(ap, tag);
    v_xmldump_writetag(fp, tag, ap);
    fprintf(fp, ">\n");
}

/*!
  @brief   Write finish XML tag.
  @ingroup xml_low
  @param   fp Stream.
  @param   tag Tag.
*/
void
v_xmldump_tag_finish(FILE *fp, char *tag)
{
    v_indent(fp);
    fprintf(fp, "</%s>\n", tag);
}

/* Internal tag writer */
static void
v_xmldump_writetag(FILE *fp, char *tag, va_list ap)
{
    char *key, *val;

    v_indent(fp);
    fprintf(fp, "<%s", tag);

    key = va_arg(ap, char *);

    while (key != NULL) {
        val = va_arg(ap, char *);
        fprintf(fp, " %s=\"%s\"", key, val);
        key = va_arg(ap, char *);
    }

    va_end(ap);
}
