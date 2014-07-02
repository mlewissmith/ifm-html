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
  @brief Destruction functions.
*/

#ifndef VARS_DESTROY_H
#define VARS_DESTROY_H

#ifdef __cplusplus
extern "C" {
#endif

extern void v_destroy(void *ptr);
extern void v_destroy_default(void (*func)(void *ptr));
extern void v_destroy_with(void *ptr, void (*func)(void *ptr));

#ifdef __cplusplus
}
#endif

#endif
