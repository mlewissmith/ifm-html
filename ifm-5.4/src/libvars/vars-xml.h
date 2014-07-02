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
  @brief XML dump functions.
*/

#ifndef VARS_XMLDUMP_H
#define VARS_XMLDUMP_H

#ifdef __cplusplus
extern "C" {
#endif

extern int v_xmldump(void *ptr, FILE *fp);
extern void v_xmldump_data(FILE *fp, char *tag, char *data);
extern int v_xmldump_file(void *ptr, char *file);
extern void v_xmldump_finish(FILE *fp);
extern void v_xmldump_start(FILE *fp);
extern void v_xmldump_string(FILE *fp, char *string);
extern void v_xmldump_tag(FILE *fp, char *tag, ...);
extern void v_xmldump_tag_start(FILE *fp, char *tag, ...);
extern void v_xmldump_tag_finish(FILE *fp, char *tag);

#ifdef __cplusplus
}
#endif

#endif
