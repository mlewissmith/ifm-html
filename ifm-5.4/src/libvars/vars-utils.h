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
  @brief Utility functions.
*/

#ifndef VARS_UTILS_H
#define VARS_UTILS_H

#define V_HEXSTRING_SIZE 20

#ifdef __cplusplus
extern "C" {
#endif

extern int v_chop(char *str);
extern void v_combine(int total, int num);
extern int *v_combine_next(void);
extern unsigned char *v_compress(unsigned char *data, unsigned long size,
                                 unsigned long *csize);
extern void v_die(char *fmt, ...);
extern void v_exception(char *fmt, ...);
extern void v_exception_hook(void (*func)(char *msg));
extern void v_fatal(char *fmt, ...);
extern void v_fatal_hook(void (*func)(char *msg));
extern char *v_hexstring(void *ptr, char *buf);
extern void v_permute(int total, int num);
extern int *v_permute_next(void);
extern void v_unavailable(char *name);
extern unsigned char *v_uncompress(unsigned char *cdata, unsigned long csize,
                                   unsigned long size);
extern void v_warn(char *fmt, ...);
extern void v_warn_internal(char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
