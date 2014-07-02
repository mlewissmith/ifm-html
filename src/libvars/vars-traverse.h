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
  @brief Traversal functions.
*/

#ifndef VARS_TRAVERSE_H
#define VARS_TRAVERSE_H

#ifdef __cplusplus
extern "C" {
#endif

extern void v_pop_traverse(void);
extern void v_push_traverse(void *ptr);
extern int v_traverse(void *ptr, int (*func)(void *ptr));
extern int v_traverse_seen(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
