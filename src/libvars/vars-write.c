/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup write Read/write functions
  @ingroup methods

  These functions perform binary I/O on objects.  See @ref persist for more
  details.
*/

/*!
  @defgroup write_high High-level functions
  @ingroup write

  These functions are for high-level use.
*/

/*!
  @defgroup write_low Low-level functions
  @ingroup write

  These functions are for use when defining new object types (see @ref
  extend).
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-hash.h"

/* File magic number */
#define MAGIC 0x28d1

/* Whether to do byte-swapping when reading */
static int v_byte_swap_flag = 0;

/*!
  @brief   Flag whether to do byte-swapping.
  @ingroup write_low
  @param   flag Byte-swapping flag.
*/
void
v_byte_swap(int flag)
{
    v_byte_swap_flag = flag;
}

/*!
  @brief   Return whether input is byte-swapped.
  @ingroup write_low
  @return  Yes or no.
*/
int
v_byte_swapped(void)
{
    return v_byte_swap_flag;
}

/*!
  @brief   Read Vars object from a stream.
  @ingroup write_high
  @param   fp Stream to read.
  @return  Pointer to Vars object.
  @retval  NULL if it failed.
*/
void *
v_read(FILE *fp)
{
    static vhash *seen = NULL;
    char *code, idbuf[10];
    static int depth = 0;
    void *data = NULL;
    vtype *t;
    int id;

    /* Declare standard types */
    v_declare();

    /* Initialise if required */
    if (depth++ == 0) {
        /* Set up data monitor */
        if (seen == NULL)
            seen = vh_create_size(1000);

        /* Read magic number */
        if (!v_read_magic(MAGIC, fp))
            goto end;

        /* Read base type name */
        if (v_read_string(fp) == NULL)
            goto end;
    }

    /* Read data ID */
    if (!v_read_long(&id, fp))
        goto end;

    if (id > 0) {
        sprintf(idbuf, "%ld", id);
        data = vh_pget(seen, idbuf);
        if (data == NULL) {
            /* Read type code */
            if ((code = v_read_string(fp)) == NULL)
                return NULL;

            /* Get associated type */
            if ((t = v_find(code)) == NULL)
                goto end;

            if (t->read == NULL)
                goto end;

            /* Read the data */
            data = (*t->read)(fp);

            /* Flag it as read */
            vh_pstore(seen, idbuf, data);
        }
    } else {
        /* Null pointer */
        data = NULL;
    }

  end:
    /* Clean up if required */
    if (--depth == 0)
        vh_empty(seen);

    return data;
}

/*!
  @brief   Read char from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_char(char *val, FILE *fp)
{
    return (int) fread(val, sizeof(char), 1, fp);
}

/*!
  @brief   Read char list from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   num No. of values to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_char_list(char *val, int num, FILE *fp)
{
    return (int) fread(val, sizeof(char), (size_t) num, fp);
}

/*!
  @brief   Read double from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_double(double *val, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(double), 1, fp);

    if (v_byte_swap_flag)
        v_swap_double(val);

    return code;
}

/*!
  @brief   Read double list from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   num No. of values to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_double_list(double *val, int num, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(double), (size_t) num, fp);

    if (v_byte_swap_flag)
        v_swap_double_array(val, num);

    return code;
}

/*!
  @brief   Read Vars object from a file.
  @ingroup write_high
  @param   file File to read.
  @return  Pointer to Vars object.
  @retval  NULL if it failed.
*/
void *
v_read_file(char *file)
{
    void *ptr = NULL;
    FILE *fp;

    if ((fp = fopen(file, "rb")) != NULL) {
        ptr = v_read(fp);
        fclose(fp);
    }

    return ptr;
}

/*!
  @brief   Read float from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_float(float *val, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(float), 1, fp);

    if (v_byte_swap_flag)
        v_swap_float(val);

    return code;
}

/*!
  @brief   Read float list from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   num No. of values to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_float_list(float *val, int num, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(float), (size_t) num, fp);

    if (v_byte_swap_flag)
        v_swap_float_array(val, num);

    return code;
}

/*!
  @brief   Read int from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_int(int *val, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(int), 1, fp);

    if (v_byte_swap_flag)
        v_swap_int((unsigned int *) val);

    return code;
}

/*!
  @brief   Read int list from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   num No. of values to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_int_list(int *val, int num, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(int), (size_t) num, fp);

    if (v_byte_swap_flag)
        v_swap_int_array((unsigned int *) val, num);

    return code;
}

/*!
  @brief   Read long from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_long(int *val, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(int), 1, fp);

    if (v_byte_swap_flag)
        v_swap_long((unsigned int *) val);

    return code;
}

/*!
  @brief   Read long list from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   num No. of values to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_long_list(int *val, int num, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(int), (size_t) num, fp);

    if (v_byte_swap_flag)
        v_swap_long_array((unsigned int *) val, num);

    return code;
}

/*!
  @brief   Read and check a magic number.
  @ingroup write_low
  @param   magic Magic number required.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_magic(unsigned short magic, FILE *fp)
{
    unsigned short file_magic = 0;
    int code = 1;

    if (!fread(&file_magic, sizeof(short), 1, fp))
        return 0;

    if (magic == file_magic) {
        v_byte_swap_flag = 0;
    } else {
        v_swap_short(&file_magic);

        if (magic == file_magic)
            v_byte_swap_flag = 1;
        else
            code = 0;
    }

    return code;
}

/*!
  @brief   Read short from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_short(short *val, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(short), 1, fp);

    if (v_byte_swap_flag)
        v_swap_short((unsigned short *) val);

    return code;
}

/*!
  @brief   Read short list from a stream.
  @ingroup write_low
  @param[out]   val Value to read.
  @param   num No. of values to read.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_read_short_list(short *val, int num, FILE *fp)
{
    int code;

    code = (int) fread(val, sizeof(short), (size_t) num, fp);

    if (v_byte_swap_flag)
        v_swap_short_array((unsigned short *) val, num);

    return code;
}

/*!
  @brief   Read a string from a stream.
  @ingroup write_low
  @param   fp Stream.
  @return  String (pointer to internal buffer).
  @retval  NULL if it failed.
*/
char *
v_read_string(FILE *fp)
{
    static vbuffer *b = NULL;
    int c;

    vb_init(b);

    do {
        if ((c = fgetc(fp)) != EOF)
            vb_putc(b, c);
        else
            return NULL;
    } while (c != '\0');

    return vb_get(b);
}

/*!
  @brief   Swap bytes in a double.
  @ingroup write_low
  @param   dp Pointer to double.
*/
void
v_swap_double(double *dp)
{
    unsigned int *lp = (unsigned int *) dp;
    unsigned int t;

    v_swap_long_array(lp, 2);
    t = lp[0];
    lp[0] = lp[1];
    lp[1] = t;
}

/*!
  @brief   Swap bytes in an array of doubles.
  @ingroup write_low
  @param   dp Pointer to double array.
  @param   n No. of entries.
*/
void
v_swap_double_array(double *dp, unsigned int n)
{
    unsigned int *lp = (unsigned int *) dp;
    unsigned int t;

    v_swap_long_array(lp, n + n);
    while (n-- > 0) {
	t = lp[0];
	lp[0] = lp[1];
	lp[1] = t;
	lp += 2;
    }
}

/*!
  @brief   Swap bytes in a float.
  @ingroup write_low
  @param   fp Pointer to float.
*/
void
v_swap_float(float *fp)
{
    unsigned int *lp = (unsigned int *) fp;
    v_swap_long(lp);
}

/*!
  @brief   Swap bytes in an array of floats.
  @ingroup write_low
  @param   fp Pointer to float array.
  @param   n No. of entries.
*/
void
v_swap_float_array(float *fp, unsigned int n)
{
    unsigned int *lp = (unsigned int *) fp;
    v_swap_long_array(lp, n);
}

/*!
  @brief   Swap bytes in an int.
  @ingroup write_low
  @param   ip Pointer to int.
*/
void
v_swap_int(unsigned int *ip)
{
    unsigned char *cp = (unsigned char *) ip;
    int t;

    t = cp[3];
    cp[3] = cp[0];
    cp[0] = t;

    t = cp[2];
    cp[2] = cp[1];
    cp[1] = t;
}

/*!
  @brief   Swap bytes in an array of ints.
  @ingroup write_low
  @param   ip Pointer to int array.
  @param   n No. of entries.
*/
void
v_swap_int_array(unsigned int *ip, unsigned int n)
{
    unsigned char *cp;
    int t;

    while (n-- > 0) {
	cp = (unsigned char *) ip;

	t = cp[3];
	cp[3] = cp[0];
	cp[0] = t;

	t = cp[2];
	cp[2] = cp[1];
	cp[1] = t;

	ip++;
    }
}

/*!
  @brief   Swap bytes in a long.
  @ingroup write_low
  @param   lp Pointer to long.
*/
void
v_swap_long(unsigned int *lp)
{
    unsigned char *cp = (unsigned char *) lp;
    int t;

    t = cp[3];
    cp[3] = cp[0];
    cp[0] = t;

    t = cp[2];
    cp[2] = cp[1];
    cp[1] = t;
}

/*!
  @brief   Swap bytes in an array of longs.
  @ingroup write_low
  @param   lp Pointer to long array.
  @param   n No. of entries.
*/
void
v_swap_long_array(unsigned int *lp, unsigned int n)
{
    unsigned char *cp;
    int t;

    while (n-- > 0) {
	cp = (unsigned char *) lp;

	t = cp[3];
	cp[3] = cp[0];
	cp[0] = t;

	t = cp[2];
	cp[2] = cp[1];
	cp[1] = t;

	lp++;
    }
}

/*!
  @brief   Swap bytes in a short.
  @ingroup write_low
  @param   sp Pointer to short.
*/
void
v_swap_short(unsigned short *sp)
{
    unsigned char *cp = (unsigned char *) sp;
    int t;

    t = cp[1];
    cp[1] = cp[0];
    cp[0] = t;
}

/*!
  @brief   Swap bytes in an array of shorts.
  @ingroup write_low
  @param   sp Pointer to short array.
  @param   n No. of entries.
*/
void
v_swap_short_array(unsigned short *sp, unsigned int n)
{
    unsigned char *cp;
    int t;

    while (n-- > 0) {
	cp = (unsigned char *) sp;
	t = cp[1];
	cp[1] = cp[0];
	cp[0] = t;
	sp++;
    }
}

/*!
  @brief   Write a pointer to a stream.
  @ingroup write_high
  @param   ptr Pointer to Vars object.
  @param   fp Stream to write.
  @return  Whether successful.
*/
int
v_write(void *ptr, FILE *fp)
{
    char *key, buf[V_HEXSTRING_SIZE];
    static vhash *seen = NULL;
    static int count = 0;
    static int depth = 0;
    int id, zero = 0;
    vtype *t;

    /* Get pointer type */
    if (ptr != NULL) {
        if ((t = v_type(ptr)) == NULL)
            return 0;
        if (t->write == NULL)
            return 0;
    }

    /* Initialise if required */
    if (depth++ == 0) {
        /* Write magic number */
        if (!v_write_magic(MAGIC, fp))
            return 0;

        /* Write base type name */
        if (!v_write_string(t->name, fp))
            return 0;

        /* Set up data monitor */
        if (seen == NULL)
            seen = vh_create_size(1000);
        count = 0;
    }

    if (ptr != NULL) {
        key = vh_pkey_buf(ptr, buf);
        id = vh_iget(seen, key);
        if (id == 0) {
            /* Flag it as written */
            id = ++count;
            vh_istore(seen, key, id);

            /* Write data ID */
            if (!v_write_long(id, fp))
                return 0;

            /* Write type code */
            if (!v_write_string(t->code, fp))
                return 0;

            /* Write the data */
            if (!(*t->write)(ptr, fp))
                return 0;
        } else {
            /* Seen before -- write old ID */
            if (!v_write_long(id, fp))
                return 0;
        }
    } else {
        /* Null pointer -- write null data ID */
        if (!v_write_long(zero, fp))
            return 0;
    }

    /* Clean up if required */
    if (--depth == 0)
        vh_empty(seen);

    return 1;
}

/*!
  @brief   Write char to a stream.
  @brief   
  @ingroup write_low
  @param   val Value to write.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_char(char val, FILE *fp)
{
    return (int) fwrite(&val, sizeof(char), 1, fp);
}

/*!
  @brief   Write char list to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   num No. of entries.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_char_list(char *val, int num, FILE *fp)
{
    return (int) fwrite(val, sizeof(char), (size_t) num, fp);
}

/*!
  @brief   Write double to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_double(double val, FILE *fp)
{
    return (int) fwrite(&val, sizeof(double), 1, fp);
}

/*!
  @brief   Write double list to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   num No. of entries.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_double_list(double *val, int num, FILE *fp)
{
    return (int) fwrite(val, sizeof(double), (size_t) num, fp);
}

/*!
  @brief   Write a pointer to a file.
  @ingroup write_high
  @param   ptr Pointer to Vars object.
  @param   file File to write.
  @return  Whether successful.
*/
int
v_write_file(void *ptr, char *file)
{
    int ok = 0;
    FILE *fp;

    if ((fp = fopen(file, "wb")) != NULL) {
        ok = v_write(ptr, fp);
        fclose(fp);
    }

    return ok;
}

/*!
  @brief   Write float to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_float(float val, FILE *fp)
{
    return (int) fwrite(&val, sizeof(float), 1, fp);
}

/*!
  @brief   Write float list to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   num No. of entries.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_float_list(float *val, int num, FILE *fp)
{
    return (int) fwrite(val, sizeof(float), (size_t) num, fp);
}

/*!
  @brief   Write int to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_int(int val, FILE *fp)
{
    return (int) fwrite(&val, sizeof(int), 1, fp);
}

/*!
  @brief   Write int list to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   num No. of entries.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_int_list(int *val, int num, FILE *fp)
{
    return (int) fwrite(val, sizeof(int), (size_t) num, fp);
}

/*!
  @brief   Write long to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_long(int val, FILE *fp)
{
    return (int) fwrite(&val, sizeof(int), 1, fp);
}

/*!
  @brief   Write long list to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   num No. of entries.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_long_list(int *val, int num, FILE *fp)
{
    return (int) fwrite(val, sizeof(int), (size_t) num, fp);
}

/*!
  @brief   Write a magic number to a stream.
  @ingroup write_low
  @param   magic Magic number.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_magic(unsigned short magic, FILE *fp)
{
    return (int) fwrite(&magic, sizeof(short), 1, fp);
}

/*!
  @brief   Write short to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_short(short val, FILE *fp)
{
    return (int) fwrite(&val, sizeof(short), 1, fp);
}

/*!
  @brief   Write short list to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   num No. of entries.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_short_list(short *val, int num, FILE *fp)
{
    return (int) fwrite(val, sizeof(short), (size_t) num, fp);
}

/*!
  @brief   Write a string to a stream.
  @ingroup write_low
  @param   val Value to write.
  @param   fp Stream.
  @return  Whether successful.
*/
int
v_write_string(char *val, FILE *fp)
{
    if (fputs(val, fp) == EOF)
        return 0;

    if (fputc('\0', fp) == EOF)
        return 0;

    return 1;
}
