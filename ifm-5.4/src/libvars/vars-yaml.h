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
  @brief YAML I/O functions.
*/

#ifndef VARS_YAML_H
#define VARS_YAML_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vars-hash.h>
#include <vars-list.h>
#include <vars-scalar.h>

extern void *v_yaml_read(FILE *fp);
extern void *v_yaml_read_file(char *file);
extern int v_yaml_write(void *ptr, FILE *fp);
extern int v_yaml_write_file(void *ptr, char *file);
extern int v_yaml_write_scalar(vscalar *s, FILE *fp);
extern int v_yaml_write_list(vlist *l, FILE *fp);
extern int v_yaml_write_double(double val, FILE *fp);
extern int v_yaml_write_int(int val, FILE *fp);
extern int v_yaml_write_string(char *val, FILE *fp);
extern int v_yaml_start(FILE *fp);
extern int v_yaml_finish(FILE *fp);
extern int v_yaml_start_list(FILE *fp, char *tag);
extern int v_yaml_finish_list(FILE *fp);
extern int v_yaml_write_hash(vhash *h, FILE *fp);
extern int v_yaml_start_hash(FILE *fp, char *tag);
extern int v_yaml_finish_hash(FILE *fp);
extern char *v_yaml_error(void);

#ifdef __cplusplus
}
#endif

#endif
