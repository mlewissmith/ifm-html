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
  @brief Read/write functions.
*/

#ifndef VARS_WRITE_H
#define VARS_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

extern void v_byte_swap(int flag);
extern int v_byte_swapped(void);
extern void *v_read(FILE *fp);
extern int v_read_char(char *val, FILE *fp);
extern int v_read_char_list(char *val, int num, FILE *fp);
extern int v_read_double(double *val, FILE *fp);
extern int v_read_double_list(double *val, int num, FILE *fp);
extern void *v_read_file(char *file);
extern int v_read_float(float *val, FILE *fp);
extern int v_read_float_list(float *val, int num, FILE *fp);
extern int v_read_int(int *val, FILE *fp);
extern int v_read_int_list(int *val, int num, FILE *fp);
extern int v_read_long(int *val, FILE *fp);
extern int v_read_long_list(int *val, int num, FILE *fp);
extern int v_read_magic(unsigned short magic, FILE *fp);
extern int v_read_short(short *val, FILE *fp);
extern int v_read_short_list(short *val, int num, FILE *fp);
extern char *v_read_string(FILE *fp);
extern void v_swap_double(double *dp);
extern void v_swap_double_array(double *dp, unsigned int n);
extern void v_swap_float(float *fp);
extern void v_swap_float_array(float *fp, unsigned int n);
extern void v_swap_int(unsigned int *ip);
extern void v_swap_int_array(unsigned int *ip, unsigned int n);
extern void v_swap_long(unsigned int *lp);
extern void v_swap_long_array(unsigned int *lp, unsigned int n);
extern void v_swap_short(unsigned short *sp);
extern void v_swap_short_array(unsigned short *sp, unsigned int n);
extern int v_write(void *ptr, FILE *fp);
extern int v_write_char(char val, FILE *fp);
extern int v_write_char_list(char *val, int num, FILE *fp);
extern int v_write_double(double val, FILE *fp);
extern int v_write_double_list(double *val, int num, FILE *fp);
extern int v_write_file(void *ptr, char *file);
extern int v_write_float(float val, FILE *fp);
extern int v_write_float_list(float *val, int num, FILE *fp);
extern int v_write_int(int val, FILE *fp);
extern int v_write_int_list(int *val, int num, FILE *fp);
extern int v_write_long(int val, FILE *fp);
extern int v_write_long_list(int *val, int num, FILE *fp);
extern int v_write_magic(unsigned short magic, FILE *fp);
extern int v_write_short(short val, FILE *fp);
extern int v_write_short_list(short *val, int num, FILE *fp);
extern int v_write_string(char *val, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
