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
  @brief Memory allocation functions and macros.
  @ingroup util_memory
*/

#ifndef VARS_MEMORY_H
#define VARS_MEMORY_H

#include <string.h>

/*! @brief Duplicate a string. */
#define V_STRDUP(str)                                                   \
        v_strdup(str)

/*! @brief Allocate some memory. */
#define V_ALLOC(type, num)                                              \
	(type *) v_malloc((size_t) ((num) * sizeof(type)))

/*! @brief Allocate zeroized memory. */
#define V_CALLOC(type, num)						\
	(type *) v_calloc(sizeof(type), (size_t) num)

/*! @brief Reallocate some memory. */
#define V_REALLOC(var, type, num)                                       \
	(type *) v_realloc((void *) var, (size_t) ((num) * sizeof(type)))

/*! @brief Deallocate memory. */
#define V_DEALLOC(var)                                                  \
	{ if ((var) != NULL) v_free(var); var = NULL; }

/*! @brief Allocate memory on the stack. */
#define V_ALLOCA(type, num)                                             \
	(type *) alloca((size_t)(num) * sizeof(type))

/*! @brief Zeroise an array. */
#define V_ZERO(var, num)                                                \
	memset((void *) var, 0, (size_t) ((num) * sizeof(*(var))))

/*! @brief Copy some memory. */
#define V_COPY(to, from, num)						\
	memcpy((void *) to, (void *) from, (size_t) ((num) * sizeof(*(to))))

/*! @brief Duplicate some memory. */
#define V_DUPLICATE(type, orig, num)					\
	(type*)	v_duplicate((void *) orig, (size_t) ((num) * sizeof(type)))

/*! @brief Allocate a 1D array. */
#define V_ALLOC_1D(var, type, d1) {                                     \
        var = V_ALLOC(type, d1);                                        \
	V_ZERO(var, d1);                                                \
}

/*! @brief Deallocate a 1D array. */
#define V_DEALLOC_1D(var, d1) {                                         \
        V_DEALLOC(var);                                                 \
}

/*! @brief Allocate a 2D array. */
#define V_ALLOC_2D(var, type, d1, d2) {                                 \
        int i1;                                                         \
        var = V_ALLOC(type *, d1);                                      \
        for (i1 = 0; i1 < d1; i1++)                                     \
            V_ALLOC_1D(var[i1], type, d2);                              \
}

/*! @brief Deallocate a 2D array. */
#define V_DEALLOC_2D(var, d1, d2) {                                     \
        int i1;                                                         \
        for (i1 = 0; i1 < d1; i1++)                                     \
            V_DEALLOC_1D(var[i1], d2);                                  \
        V_DEALLOC(var);                                                 \
}

/*! @brief Allocate a 3D array. */
#define V_ALLOC_3D(var, type, d1, d2, d3) {                             \
        int i2;                                                         \
        var = V_ALLOC(type **, d1);                                     \
        for (i2 = 0; i2 < d1; i2++)                                     \
            V_ALLOC_2D(var[i2], type, d2, d3);                          \
}

/*! @brief Deallocate a 3D array. */
#define V_DEALLOC_3D(var, d1, d2, d3) {                                 \
        int i2;                                                         \
        for (i2 = 0; i2 < d1; i2++)                                     \
            V_DEALLOC_2D(var[i2], d2, d3);                              \
        V_DEALLOC(var);                                                 \
}

/*! @brief Allocate a 4D array. */
#define V_ALLOC_4D(var, type, d1, d2, d3, d4) {                         \
        int i3;                                                         \
        var = V_ALLOC(type ***, d1);                                    \
        for (i3 = 0; i3 < d1; i3++)                                     \
            V_ALLOC_3D(var[i3], type, d2, d3, d4);                      \
}

/*! @brief Deallocate a 4D array. */
#define V_DEALLOC_4D(var, d1, d2, d3, d4) {                             \
        int i3;                                                         \
        for (i3 = 0; i3 < d1; i3++)                                     \
            V_DEALLOC_3D(var[i3], d2, d3, d4);                          \
        V_DEALLOC(var);                                                 \
}

/*! @brief Allocate temporary formatted string. */
#define V_ALLOCA_FMT(str, fmt) {                                        \
	int need, size = 80;                                            \
        va_list ap;							\
        while (1) {                                                     \
	    str = V_ALLOCA(char, size);                                 \
	    va_start(ap, fmt);						\
	    need = vsnprintf(str, (size_t) size, fmt, ap) + 10;         \
	    va_end(ap);							\
	    if (need > size)                                            \
	        size = need;                                            \
            else                                                        \
	        break;							\
	}								\
}

#ifdef __cplusplus
extern "C" {
#endif

extern void *v_calloc(size_t nmemb, size_t size);
extern void *v_duplicate(const void *orig, size_t size);
extern void v_free(void *ptr);
extern void *v_malloc(size_t size);
extern void v_memerr_hook(void (*func)(size_t size));
extern void *v_realloc(void *ptr, size_t size);
extern char *v_strdup(const char *str);

#ifdef __cplusplus
}
#endif

#endif
