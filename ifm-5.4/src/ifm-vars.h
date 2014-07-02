/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Variable function header */

#ifndef IFM_VARS_H
#define IFM_VARS_H

#define VAR_DEF(var)  (var_get(var) != NULL)

/* Advertised functions */
extern void add_style(char *name);
extern char *current_style(void);
extern vlist *current_styles(void);
extern void load_styles(void);
extern void pop_style(char *name);
extern void push_style(char *name);
extern void ref_style(char *name);
extern void set_style(char *name);
extern void set_style_list(vlist *list);
extern int var_changed(char *id);
extern char *var_colour(char *id);
extern vscalar *var_get(char *id);
extern int var_int(char *id);
extern void var_list(void);
extern double var_real(char *id);
extern void var_set(char *driver, char *id, vscalar *val);
extern char *var_string(char *id);

#endif
