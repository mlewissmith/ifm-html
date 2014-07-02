/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup buffer Buffers
  @ingroup types

  A buffer is essentially just a character string which grows automatically
  as data is appended to it.
*/

/*!
  @defgroup buffer_create Creating and destroying buffers
  @ingroup buffer

  These functions create and destroy buffers.
*/

/*!
  @defgroup buffer_modify Adding to buffers
  @ingroup buffer

  These functions modify the contents of buffers.
*/

/*!
  @defgroup buffer_io Reading and writing buffers
  @ingroup buffer

  These functions perform file I/O with buffers.
*/

/*!
  @defgroup buffer_access Accessing buffer contents
  @ingroup buffer

  These functions query the contents of buffers.
*/

/*!
  @defgroup buffer_misc Other buffer functions
  @ingroup buffer

  These are miscellaneous buffer functions.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-macros.h"
#include "vars-memory.h"

#ifndef BUFFER_DEFAULT_SIZE
#define BUFFER_DEFAULT_SIZE 512
#endif

#ifndef BUFFER_DEFAULT_PRINT
#define BUFFER_DEFAULT_PRINT 128
#endif

#define PACK(b, type, val, infield, outfield) {                         \
        packer p;                                                       \
        p.infield = val;                                                \
        APPEND(b, p.outfield, sizeof(type));                            \
}

#define UNPACK(b, type, val, infield, outfield) {                       \
        type *ptr = val;                                                \
        packer p;                                                       \
        int i;                                                          \
        for (i = 0; i < sizeof(type); i++)                              \
            p.infield[i] = b->buf[b->rpos++];                           \
        *ptr = p.outfield;                                              \
}

#define APPEND(b, str, size) {                                          \
        int i;                                                          \
        REQUIRE(b, b->len + size);                                      \
        for (i = 0; i < size; i++)                                      \
            b->buf[b->len++] = str[i];                                  \
}

#define REQUIRE(b, need) if (b->size <= need) vb_require(b, need)

/* Type definition */
struct v_buffer {
    struct v_header id;         /* Type marker */
    char *buf;                  /* Buffer data */
    unsigned int bufsize;       /* Initial buffer size */
    unsigned int len;           /* Data length */
    unsigned int size;          /* Current buffer size */
    unsigned int rpos;          /* Current read position */
    unsigned int pmax;          /* Max. chars to print */
};

typedef union packer_st {
    short sval;
    int ival;
    float fval;
    double dval;
    char cval[8];
} packer;

/* Type variable */
vtype *vbuffer_type = NULL;

/* Default buffer allocation size */
static unsigned int bufsize = BUFFER_DEFAULT_SIZE;

/* Internal functions */
static int vb_xmldump(vbuffer *b, FILE *fp);

/*!
  @brief   Set the default initial buffer size used by vb_create().
  @ingroup buffer_misc
  @param   size Initial size.
*/
void 
vb_bufsize(unsigned int size)
{
    bufsize = V_MAX(size, 1);
}

/* Check if pointer is a buffer */
int
vb_check(void *ptr)
{
    return (ptr != NULL && v_type(ptr) == vbuffer_type);
}

/*!
  @brief   Chop the last character from a buffer and return it.
  @ingroup buffer_misc
  @param   b Buffer.
  @return  Last character.
  @retval  EOF if the buffer is empty.
*/
int
vb_chop(vbuffer *b)
{
    int c = EOF;

    VB_CHECK(b);

    if (b->len > 0) {
        c = b->buf[b->len - 1];
        b->buf[--b->len] = '\0';
    }

    return c;
}

/*!
  @brief   Copy a buffer.
  @ingroup buffer_create
  @param   b Buffer.
  @return  Copy of the buffer.
*/
vbuffer *
vb_copy(vbuffer *b)
{
    vbuffer *copy;

    copy = vb_create_size(b->bufsize);
    vb_store(copy, b->buf);

    return copy;
}

/*!
  @brief   Create a buffer.
  @ingroup buffer_create
  @return  New buffer.
*/
vbuffer *
vb_create(void)
{
    return vb_create_size(bufsize);
}

/*!
  @brief   Create a buffer with a given initial size.
  @ingroup buffer_create
  @param   size Initial size.
  @return  New buffer.
*/
vbuffer *
vb_create_size(unsigned int size)
{
    static vheader *id = NULL;
    vbuffer *b;

    if (id == NULL) {
        vb_declare();
        id = v_header(vbuffer_type);
    }

    b = V_ALLOC(vbuffer, 1);

    b->id = *id;
    b->len = 0;
    b->size = b->bufsize = V_MAX(size, 1);
    b->rpos = 0;
    b->pmax = BUFFER_DEFAULT_PRINT;
    b->buf = V_ALLOC(char, b->size);
    b->buf[0] = '\0';

    return b;
}

/* Declare buffer type */
vtype *
vb_declare(void)
{
    if (vbuffer_type == NULL) {
        vbuffer_type = v_create("BUFFER", "B");
	v_create_func(vbuffer_type, (void *(*)()) vb_create);
        v_copy_func(vbuffer_type, (void *(*)()) vb_copy);
        v_read_func(vbuffer_type, (void *(*)()) vb_read);
        v_write_func(vbuffer_type, vb_write);
        v_freeze_func(vbuffer_type, vb_freeze);
        v_thaw_func(vbuffer_type, (void *(*)()) vb_thaw);
        v_print_func(vbuffer_type, vb_print);
        v_destroy_func(vbuffer_type, vb_destroy);
        v_traverse_func(vbuffer_type, vb_traverse);
        v_xmldump_func(vbuffer_type, vb_xmldump);
    }

    return vbuffer_type;
}

/*!
  @brief   Deallocate a buffer.
  @ingroup buffer_create
  @param   b Buffer to destroy.
*/
void 
vb_destroy(vbuffer *b)
{
    VB_CHECK(b);
    V_DEALLOC(b->buf);
    V_DEALLOC(b);
}

/*!
  @brief   Empty a buffer's contents.
  @ingroup buffer_create
  @param   b Buffer to empty.
*/
void
vb_empty(vbuffer *b)
{
    VB_CHECK(b);

    if (b->size > b->bufsize)
        b->buf = V_REALLOC(b->buf, char, b->bufsize);

    b->size = b->bufsize;
    b->buf[0] = '\0';
    b->len = 0;
}

/*!
  @brief   Write buffer string to a stream.
  @ingroup buffer_io
  @param   b Buffer.
  @param   fp Output stream.
  @return  Whether write was successful.
*/
int
vb_fputs(vbuffer *b, FILE *fp)
{
    VB_CHECK(b);

    if (fwrite(b->buf, sizeof(char), b->len, fp) != b->len)
        return 0;

    return 1;
}

/* Freeze a buffer to a stream */
int
vb_freeze(vbuffer *b, FILE *fp)
{
    VB_CHECK(b);

    v_freeze_start(fp);
    v_freeze_string(b->buf, fp);
    v_freeze_finish(fp);

    return 1;
}

/*!
  @brief   Read from a stream into a buffer.
  @ingroup buffer_io
  @param   b Buffer.
  @param   delim Delimiter string.
  @param   fp Input stream.
  @return  Bytes read in, as a string.
  @retval  NULL if no more input.

  Read bytes from the specified stream into to a buffer until a character
  from \c delim is seen, and return the bytes read as a string. This
  overwrites the contents of the buffer. The delimiter character is the
  last character in the returned string. For example, setting \c delim to a
  newline character would read input line-by-line, and setting it to \c
  NULL would slurp the whole input in one big lump. Returns \c NULL if
  there is no more input to read.
*/
char *
vb_fscan(vbuffer *b, char *delim, FILE *fp)
{
    int c, len, bytes = 0;

    VB_CHECK(b);

    /* Flush contents */
    b->buf[0] = '\0';
    b->len = 0;

    /* Read data */
    len = b->len;
    while ((c = fgetc(fp)) != EOF) {
	bytes++;
	if (b->len + 1 == b->size)
	    REQUIRE(b, b->len + 1);

	b->buf[b->len++] = c;
        if (delim != NULL && strchr(delim, c) != NULL)
	    break;
    }

    b->buf[b->len] = '\0';

    return (bytes ? &b->buf[len] : NULL);
}

/*!
  @brief   Return a buffer's string.
  @ingroup buffer_access
  @param   b Buffer.
  @return  String.

  @note
  This function returns a pointer to the internal buffer, which may be
  modified by other calls.
*/
char *
vb_get(vbuffer *b)
{
    VB_CHECK(b);
    return b->buf;
}

/*!
  @brief   Return length of a buffer string.
  @ingroup buffer_access
*/
int 
vb_length(vbuffer *b)
{
    VB_CHECK(b);
    return b->len;
}

/* Pack values into a buffer */
int
vb_pack(vbuffer *b, char *fmt, ...)
{
    va_list ap;
    char *str;

    VB_CHECK(b);

    va_start(ap, fmt);
    while (*fmt != '\0') {
        switch (*fmt++) {
        case 'a':
            str = va_arg(ap, char *);
            APPEND(b, str, strlen(str) + 1);
            break;
        case 'c':
            REQUIRE(b, b->len + 1);
            b->buf[b->len++] = va_arg(ap, int);
            break;
        case 'd':
            PACK(b, double, va_arg(ap, double), dval, cval);
            break;
        case 'f':
            PACK(b, float, va_arg(ap, double), fval, cval);
            break;
        case 'i':
            PACK(b, int, va_arg(ap, int), ival, cval);
            break;
        case 's':
            PACK(b, short, va_arg(ap, int), sval, cval);
            break;
        default:
            va_end(ap);
            return 0;
        }
    }

    va_end(ap);
    return 1;
}

/* Print contents of a buffer */
void
vb_print(vbuffer *b, FILE *fp)
{
    int i, nlook, noprint, binary, min, max, numchars;
    char c;

    VB_CHECK(b);

    v_print_start();
    v_push_indent();

    v_print_type(vbuffer_type, b, fp);

    /* See if it looks like binary data */
    nlook = V_MIN(b->len, 50);
    for (i = 0, noprint = 0; i < nlook; i++) {
        c = b->buf[i];
        if (c != '\n' && c != '\t' && !isprint(c))
            noprint++;
    }

    binary = ((double) noprint / nlook > 0.2);

    /* Do printing */
    if (b->pmax > 0)
        numchars = V_MIN(b->len, b->pmax);
    else
        numchars = b->len;

    if (!binary) {
        v_indent(fp);

        for (i = 0; i < numchars; i++) {
            c = b->buf[i];
            if (c == '\n') {
                fputc('\n', fp);
                if (i < numchars - 1)
                    v_indent(fp);
            } else if (isprint(c)) {
                fputc(c, fp);
            } else {
                fputc('?', fp);
            }
        }

        if (b->len > numchars)
            fprintf(fp, "...\n");
        else if (c != '\n')
            fprintf(fp, "\n");
    } else {
        min = 0;
        max = V_MIN(min + 16, numchars);

        while (1) {
            v_indent(fp);
            fprintf(fp, "%08x: ", min);

            for (i = min; i < min + 16; i++) {
                if (i < max)
                    fprintf(fp, "%02x", b->buf[i]);
                else
                    fprintf(fp, "  ");
                if (i % 2 != 0)
                    fputc(' ', fp);
            }

            fputc(' ', fp);
            for (i = min; i < max; i++)
                fputc(isprint(b->buf[i]) ? b->buf[i] : '.', fp);
            fputc('\n', fp);

            if (max == numchars)
                break;

            min = max;
            max = V_MIN(max + 16, numchars);
        }
    }

    v_pop_indent();
    v_print_finish();
}

/* Set max. printing chars */
void
vb_print_max(vbuffer *b, unsigned int chars)
{
    VB_CHECK(b);
    b->pmax = chars;
}

/*!
  @brief   Do a \c printf() onto a buffer.
  @ingroup buffer_modify
  @param   b Buffer.
  @param   fmt Format string.
*/
void
vb_printf(vbuffer *b, char *fmt, ...)
{
    int n, size = 100;
    va_list ap;

    VB_CHECK(b);

    while (1) {
        REQUIRE(b, b->len + size);
        va_start(ap, fmt);
        n = vsnprintf(b->buf + b->len, (size_t) size, fmt, ap);
        va_end(ap);

        if (n == size - 1) {
            /* Could be due to buggy vsnprintf() */
            size += 10;
        } else if (n > -1 && n < size) {
            /* Everything written */
            b->len += n;
            return;
        } else if (n > -1) {
            /* Not big enough, but correct size returned */
            size = n + 1;
        } else {
            /* Not big enough */
            size *= 2;
        }
    }
}

/*!
  @brief   Append a character to a buffer.
  @ingroup buffer_modify
  @param   b Buffer.
  @param   c Character to append.
*/
void
vb_putc(vbuffer *b, char c)
{
    VB_CHECK(b);
    REQUIRE(b, b->len + 1);
    b->buf[b->len++] = c;
    b->buf[b->len] = '\0';
}

/*!
  @brief   Append a string to a buffer.
  @ingroup buffer_modify
  @param   b Buffer.
  @param   string String to append.
*/
void 
vb_puts(vbuffer *b, char *string)
{
    VB_CHECK(b);
    APPEND(b, string, strlen(string));
    b->buf[b->len] = '\0';
}

/* Read buffer from a stream */
vbuffer *
vb_read(FILE *fp)
{
    int size, len;
    vbuffer *b;

    /* Read initial buffer size */
    if (!v_read_long(&size, fp))
        return NULL;

    /* Read buffer length */
    if (!v_read_long(&len, fp))
        return NULL;

    /* Read buffer string */
    b = vb_create_size(size);
    REQUIRE(b, len);
    if (fread(b->buf, sizeof(char), (size_t) len, fp) != (size_t) len)
        return NULL;

    b->len = len;

    return b;
}

/*!
  @brief   Increase buffer allocation to meet requirement.
  @ingroup buffer_misc
  @param   b Buffer.
  @param   size Size required.

  Tell a buffer to grow until it is at least as large as the given size. If
  the buffer is already large enough, nothing is done. If not, the buffer
  will increase in multiples of its initial size until large enough.
*/
void 
vb_require(vbuffer *b, unsigned int size)
{
    VB_CHECK(b);

    if (b->size > size)
        return;
    while (b->size <= size)
        b->size += b->bufsize;
    b->buf = V_REALLOC(b->buf, char, b->size);
}

/*!
  @brief   Set a buffer to a string.
  @ingroup buffer_modify
  @param   b Buffer.
  @param   string String to set it to.
*/
void
vb_store(vbuffer *b, char *string)
{
    int len = strlen(string);

    VB_CHECK(b);
    REQUIRE(b, len);
    strcpy(b->buf, string);
    b->len = len;
}

/* Thaw a buffer from file */
vbuffer *
vb_thaw(FILE *fp)
{
    vbuffer *b;

    v_thaw_start();

    if (!v_thaw_follow(fp, V_TOKEN_STRING, "buffer string"))
        goto fail;

    b = vb_create();
    vb_store(b, v_thaw_svalue);

    v_thaw_finish();

    return b;

  fail:
    v_thaw_finish();
    return NULL;
}

/* Traverse a buffer */
int
vb_traverse(vbuffer *b, int (*func)(void *ptr))
{
    int val;

    VB_CHECK(b);

    if ((val = func(b)) != 0)
        return val;

    v_push_traverse(b);
    v_pop_traverse();

    return 0;
}

/* Unpack values from a buffer */
int
vb_unpack(vbuffer *b, char *fmt, ...)
{
    va_list ap;
    char *str;

    VB_CHECK(b);

    va_start(ap, fmt);
    while (*fmt != '\0') {
        switch (*fmt++) {
        case 'a':
            str = va_arg(ap, char *);
            while (b->buf[b->rpos] != '\0')
                *str++ = b->buf[b->rpos++];
            *str = '\0';
            b->rpos++;
            break;
        case 'c':
            str = va_arg(ap, char *);
            *str = b->buf[b->rpos++];
            break;
        case 'd':
            UNPACK(b, double, va_arg(ap, double *), cval, dval);
            break;
        case 'f':
            UNPACK(b, float, va_arg(ap, float *), cval, fval);
            break;
        case 'i':
            UNPACK(b, int, va_arg(ap, int *), cval, ival);
            break;
        case 's':
            UNPACK(b, short, va_arg(ap, short *), cval, sval);
            break;
        default:
            va_end(ap);
            return 0;
        }
    }

    va_end(ap);
    return 1;
}

/* Write buffer to a stream */
int
vb_write(vbuffer *b, FILE *fp)
{
    VB_CHECK(b);

    /* Write initial buffer size */
    if (!v_write_long(b->size, fp))
        return 0;

    /* Write buffer length */
    if (!v_write_long(b->len, fp))
        return 0;

    /* Write buffer string */
    return vb_fputs(b, fp);
}

/* Dump XML buffer to a stream */
static int
vb_xmldump(vbuffer *b, FILE *fp)
{
    VB_CHECK(b);

    v_xmldump_start(fp);
    v_xmldump_data(fp, "string", vb_get(b));
    v_xmldump_finish(fp);

    return 1;
}
