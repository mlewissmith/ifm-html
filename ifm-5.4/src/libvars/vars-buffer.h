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
  @brief Buffer functions and macros.
  @ingroup buffer
*/

#ifndef VARS_BUFFER_H
#define VARS_BUFFER_H

#include <stdarg.h>
#include <vars-type.h>
#include <vars-memory.h>

/*! @brief Abort if a pointer is not a buffer. */
#define VB_CHECK(ptr)           V_CHECK(ptr, vb_check, "BUFFER")

/*! @brief Get a copy of a buffer's string */
#define vb_getcopy(b)           V_STRDUP(vb_get(b))

/*!
  @brief Read and return a line from a stream

  Like vb_fscan() with \c delim set to "\n" -- i.e. it reads input
  line-by-line.
*/
#define vb_fgets(b, fp)         vb_fscan(b, "\n", fp)

/*!
  @brief Read and return a line from stdin

  Like vb_fscan() with \c delim set to "\n" and \c fp set to stdin.
  However, unlike the \c gets(3) call it is similar to, it doesn't strip
  the newline character from the input.
*/
#define vb_gets(b)              vb_fscan(b, "\n", stdin)

/*! @brief Create or empty a buffer. */
#define vb_init(b) if (b == NULL) b = vb_create(); else vb_empty(b)

#ifndef V_BUF_SIZE
/*! @brief Default buffer size, if not already defined. */
#define V_BUF_SIZE 80
#endif

#ifndef V_BUF_NAME
/*! @brief Name of default buffer, if not already defined. */
#define V_BUF_NAME buffer
#endif

/*! @brief Declare a buffer with the default name. */
#define V_BUF_DECL V_NBUF_DECL(V_BUF_NAME)

/*! @brief Set the buffer to be empty. */
#define V_BUF_INIT V_NBUF_INIT(V_BUF_NAME)

/*! @brief Set the buffer to contain a string. */
#define V_BUF_SET(val) V_NBUF_SET(V_BUF_NAME, val)

/*! @brief Append a string to the buffer. */
#define V_BUF_ADD(val) V_NBUF_ADD(V_BUF_NAME, val)

/*! @brief Append a string (with an optional delimiter) to the buffer. */
#define V_BUF_APPEND(delim, str) V_NBUF_APPEND(V_BUF_NAME, delim, str)

/*! @brief Chop last character off the buffer string. */
#define V_BUF_CHOP V_NBUF_CHOP(V_BUF_NAME)

/*! @brief Get string value from a buffer. */
#define V_BUF_VAL V_NBUF_VAL(V_BUF_NAME)

/*! @brief Read a line into the buffer and return it. */
#define V_BUF_FGETS(fp) V_NBUF_FGETS(V_BUF_NAME, fp)

/*! @brief Print into the buffer using stdargs. */
#define V_BUF_FMT(fmt, val) V_NBUF_FMT(V_BUF_NAME, fmt, val)

/*! @brief Set buffer in a sprintf() manner with 1 argument. */
#define V_BUF_SET1(fmt, v1)						\
	V_NBUF_SET1(V_BUF_NAME, fmt, v1)

/*! @brief Set buffer in a sprintf() manner with 2 arguments. */
#define V_BUF_SET2(fmt, v1, v2)						\
	V_NBUF_SET2(V_BUF_NAME, fmt, v1, v2)

/*! @brief Set buffer in a sprintf() manner with 3 arguments. */
#define V_BUF_SET3(fmt, v1, v2, v3)					\
	V_NBUF_SET3(V_BUF_NAME, fmt, v1, v2, v3)

/*! @brief Set buffer in a sprintf() manner with 4 arguments. */
#define V_BUF_SET4(fmt, v1, v2, v3, v4)					\
	V_NBUF_SET4(V_BUF_NAME, fmt, v1, v2, v3, v4)

/*! @brief Set buffer in a sprintf() manner with 5 arguments. */
#define V_BUF_SET5(fmt, v1, v2, v3, v4, v5)				\
	V_NBUF_SET5(V_BUF_NAME, fmt, v1, v2, v3, v4, v5)

/*! @brief Set buffer in a sprintf() manner with 6 arguments. */
#define V_BUF_SET6(fmt, v1, v2, v3, v4, v5, v6)				\
	V_NBUF_SET6(V_BUF_NAME, fmt, v1, v2, v3, v4, v5, v6)

/*! @brief Append to buffer in a sprintf() manner with 1 argument. */
#define V_BUF_ADD1(fmt, v1)						\
	V_NBUF_ADD1(V_BUF_NAME, fmt, v1)

/*! @brief Append to buffer in a sprintf() manner with 2 arguments. */
#define V_BUF_ADD2(fmt, v1, v2)						\
	V_NBUF_ADD2(V_BUF_NAME, fmt, v1, v2)

/*! @brief Append to buffer in a sprintf() manner with 3 arguments. */
#define V_BUF_ADD3(fmt, v1, v2, v3)					\
	V_NBUF_ADD3(V_BUF_NAME, fmt, v1, v2, v3)

/*! @brief Append to buffer in a sprintf() manner with 4 arguments. */
#define V_BUF_ADD4(fmt, v1, v2, v3, v4)					\
	V_NBUF_ADD4(V_BUF_NAME, fmt, v1, v2, v3, v4)

/*! @brief Append to buffer in a sprintf() manner with 5 arguments. */
#define V_BUF_ADD5(fmt, v1, v2, v3, v4, v5)				\
	V_NBUF_ADD5(V_BUF_NAME, fmt, v1, v2, v3, v4, v5)

/*! @brief Append to buffer in a sprintf() manner with 6 arguments. */
#define V_BUF_ADD6(fmt, v1, v2, v3, v4, v5, v6)				\
	V_NBUF_ADD6(V_BUF_NAME, fmt, v1, v2, v3, v4, v5, v6)

/*! @brief Declare a named buffer. */
#define V_NBUF_DECL(b) static vbuffer *b = NULL

/*! @brief Create the named buffer if required. */
#define V_NBUF_CREATE(b) (b = (b == NULL ? vb_create_size(V_BUF_SIZE) : b))

/*! @brief Set the named buffer to be empty. */
#define V_NBUF_INIT(b) (V_NBUF_CREATE(b), vb_empty(b))

/*! @brief Set the named buffer to contain a string. */
#define V_NBUF_SET(b, val) (V_NBUF_INIT(b), V_NBUF_ADD(b, val))

/*! @brief Append a string to the named buffer. */
#define V_NBUF_ADD(b, val) (vb_puts(b, val), V_NBUF_VAL(b))

/*! @brief Append a string (with an optional delimiter) to the named buffer. */
#define V_NBUF_APPEND(b, delim, str) do {                               \
        if (strlen(V_NBUF_VAL(b)) > 0)                                  \
            vb_puts(b, delim == NULL ? ", " : delim);                   \
        vb_puts(b, str);                                                \
} while (0)

/*! @brief Chop last character off a named buffer string. */
#define V_NBUF_CHOP(b) (V_NBUF_CREATE(b), vb_chop(b))

/*! @brief Get string value from a named buffer. */
#define V_NBUF_VAL(b) (V_NBUF_CREATE(b), vb_get(b))

/*! @brief Read a line into the named buffer and return it. */
#define V_NBUF_FGETS(b, fp) (V_NBUF_CREATE(b), vb_fgets(b, fp))

/*! @brief Print into a named buffer using stdargs. */
#define V_NBUF_FMT(b, fmt, val) {                                       \
        char *_str;                                                     \
        V_ALLOCA_FMT(_str, fmt);                                        \
        V_NBUF_INIT(b);                                                 \
        V_NBUF_ADD(b, _str);                                            \
        val = V_NBUF_VAL(b);                                            \
}

/*! @brief Set named buffer in a sprintf() manner with 1 argument. */
#define V_NBUF_SET1(b, fmt, v1)                                         \
	(V_NBUF_INIT(b), V_NBUF_ADD1(b, fmt, v1))

/*! @brief Set named buffer in a sprintf() manner with 2 arguments. */
#define V_NBUF_SET2(b, fmt, v1, v2)					\
	(V_NBUF_INIT(b), V_NBUF_ADD2(b, fmt, v1, v2))

/*! @brief Set named buffer in a sprintf() manner with 3 arguments. */
#define V_NBUF_SET3(b, fmt, v1, v2, v3)                                 \
	(V_NBUF_INIT(b), V_NBUF_ADD3(b, fmt, v1, v2, v3))

/*! @brief Set named buffer in a sprintf() manner with 4 arguments. */
#define V_NBUF_SET4(b, fmt, v1, v2, v3, v4)				\
	(V_NBUF_INIT(b), V_NBUF_ADD4(b, fmt, v1, v2, v3, v4))

/*! @brief Set named buffer in a sprintf() manner with 5 arguments. */
#define V_NBUF_SET5(b, fmt, v1, v2, v3, v4, v5)                         \
	(V_NBUF_INIT(b), V_NBUF_ADD5(b, fmt, v1, v2, v3, v4, v5))

/*! @brief Set named buffer in a sprintf() manner with 6 arguments. */
#define V_NBUF_SET6(b, fmt, v1, v2, v3, v4, v5, v6)			\
	(V_NBUF_INIT(b), V_NBUF_ADD6(b, fmt, v1, v2, v3, v4, v5, v6))

/*! @brief Append to named buffer in a sprintf() manner with 1 argument. */
#define V_NBUF_ADD1(b, fmt, v1)                                         \
	(vb_printf(b, fmt, v1), V_NBUF_VAL(b))

/*! @brief Append to named buffer in a sprintf() manner with 2 arguments. */
#define V_NBUF_ADD2(b, fmt, v1, v2)					\
	(vb_printf(b, fmt, v1, v2), V_NBUF_VAL(b))

/*! @brief Append to named buffer in a sprintf() manner with 3 arguments. */
#define V_NBUF_ADD3(b, fmt, v1, v2, v3)                                 \
	(vb_printf(b, fmt, v1, v2, v3), V_NBUF_VAL(b))

/*! @brief Append to named buffer in a sprintf() manner with 4 arguments. */
#define V_NBUF_ADD4(b, fmt, v1, v2, v3, v4)				\
	(vb_printf(b, fmt, v1, v2, v3, v4), V_NBUF_VAL(b))

/*! @brief Append to named buffer in a sprintf() manner with 5 arguments. */
#define V_NBUF_ADD5(b, fmt, v1, v2, v3, v4, v5)                         \
	(vb_printf(b, fmt, v1, v2, v3, v4, v5), V_NBUF_VAL(b))

/*! @brief Append to named buffer in a sprintf() manner with 6 arguments. */
#define V_NBUF_ADD6(b, fmt, v1, v2, v3, v4, v5, v6)			\
	(vb_printf(b, fmt, v1, v2, v3, v4, v5, v6), V_NBUF_VAL(b))

/*! @brief Buffer type. */
typedef struct v_buffer vbuffer;

#ifdef __cplusplus
extern "C" {
#endif

extern void vb_bufsize(unsigned int size);
extern int vb_check(void *ptr);
extern int vb_chop(vbuffer *b);
extern vbuffer *vb_copy(vbuffer *b);
extern vbuffer *vb_create(void);
extern vbuffer *vb_create_size(unsigned int size);
extern vtype *vb_declare(void);
extern void vb_destroy(vbuffer *b);
extern void vb_empty(vbuffer *b);
extern int vb_fputs(vbuffer *b, FILE *fp);
extern int vb_freeze(vbuffer *b, FILE *fp);
extern char *vb_fscan(vbuffer *b, char *delim, FILE *fp);
extern char *vb_get(vbuffer *b);
extern int vb_length(vbuffer *b);
extern int vb_pack(vbuffer *b, char *fmt, ...);
extern void vb_print(vbuffer *b, FILE *fp);
extern void vb_print_max(vbuffer *b, unsigned int chars);
extern void vb_printf(vbuffer *b, char *fmt, ...);
extern void vb_putc(vbuffer *b, char c);
extern void vb_puts(vbuffer *b, char *string);
extern vbuffer *vb_read(FILE *fp);
extern void vb_require(vbuffer *b, unsigned int size);
extern void vb_store(vbuffer *b, char *string);
extern vbuffer *vb_thaw(FILE *fp);
extern int vb_traverse(vbuffer *b, int (*func)(void *ptr));
extern int vb_unpack(vbuffer *b, char *fmt, ...);
extern int vb_write(vbuffer *b, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
