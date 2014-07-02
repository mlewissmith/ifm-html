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
  @brief Debugging functions and macros.
  @ingroup debug
*/

#ifndef VARS_DEBUG_H
#define VARS_DEBUG_H

/* Debugging flags */
#define V_DBG_MEMORY            0x0001
#define V_DBG_PTRTYPE           0x0002
#define V_DBG_PTRALLOC          0x0004
#define V_DBG_PROFILE           0x0008
#define V_DBG_INTERN            0x0010
#define V_DBG_USER              0x0020
#define V_DBG_ALL               0xffff

#define V_DBG_PTRS              (V_DBG_PTRTYPE | V_DBG_PTRALLOC)

/* Memory allocation flags */
#define V_MEM_MALLOC  1
#define V_MEM_REALLOC 2
#define V_MEM_FREE    3

/* Pointer checking macros */
#ifdef __GNUC__
#define V_CHECK(ptr, func, name) {                                      \
    if (V_DEBUG(V_DBG_PTRALLOC) && !v_allocated(ptr))                   \
        v_fatal("%s() (%s, line %d): use of non-allocated %s pointer",  \
                __FUNCTION__, __FILE__, __LINE__, name);                \
    if (V_DEBUG(V_DBG_PTRTYPE) && !func(ptr))                           \
        v_fatal("%s() (%s, line %d): expected %s pointer but got %s",   \
                __FUNCTION__, __FILE__, __LINE__, name, v_ptrname(ptr)); \
}
#else
#define V_CHECK(ptr, func, name) {                                      \
    if (V_DEBUG(V_DBG_PTRALLOC) && !v_allocated(ptr))                   \
        v_fatal("%s, line %d: use of non-allocated %s pointer",         \
                __FILE__, __LINE__, name);                              \
    if (V_DEBUG(V_DBG_PTRTYPE) && !func(ptr))                           \
        v_fatal("%s, line %d: expected %s pointer but got %s",          \
                __FILE__, __LINE__, name, v_ptrname(ptr));             \
}
#endif

/*! @brief Test a debugging flag. */
#define v_debugging(flag) V_DEBUG(flag)

/* Internal debugging flag macro */
#ifndef V_DEBUG
#define V_DEBUG(flag) (v_debug_flags & (flag))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Debugging stuff */
extern int v_debug_flags, v_memory_track, v_memory_allocated;

extern int v_allocated(void *ptr);
extern void v_debug(int flag);
extern void v_debug_env(void);
extern void v_nodebug(int flag);
extern void v_profile(char *fmt, ...);
extern int v_times(double *real, double *user, double *sys);
extern void v_memchange(int type, void *ptr, void *oldptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif
