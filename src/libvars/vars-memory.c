/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup util_memory Memory allocation functions
  @ingroup util
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include "vars-config.h"
#include "vars-copy.h"
#include "vars-debug.h"
#include "vars-destroy.h"
#include "vars-hash.h"
#include "vars-memory.h"
#include "vars-print.h"

#define UPDATE_TABLE(table, ptr) do {                                   \
        if (table != NULL) {                                            \
            if (ptr != table) {                                         \
                key = vh_pkey_buf(ptr, kbuf);                           \
                vh_delete(table, key);                                  \
            } else {                                                    \
                table = NULL;                                           \
            }                                                           \
        }                                                               \
} while (0)

/* Memory error function */
static void (*memerr_func)(size_t size) = NULL;

/*!
  @brief   Allocate some zeroized memory.
  @ingroup util_memory
  @param   nmemb No of members in new array
  @param   size No. of bytes per member of array
  @return  Allocated memory.
  @see     v_memerr_hook()
*/
void*
v_calloc(size_t nmemb, size_t size)
{
    size_t product = size * nmemb;

    /* Allocate the memory */
    void *ptr = v_malloc(product);

    /* Wipe the memory */
    memset(ptr, 0, product);

    return ptr;
}

/*!
  @brief   Duplicate memory.
  @ingroup util_memory
  @param   orig Pointer to original.
  @param   size Size in bytes.
  @return  Pointer to the copy.
*/
void *
v_duplicate(const void *orig, size_t size)
{
    void *copy = v_malloc(size);
    return memcpy(copy, orig, size);
}

/*!
  @brief   Free memory.
  @ingroup util_memory
  @param   ptr Pointer.
*/
void
v_free(void *ptr)
{
    /*@-shadow@*/
    extern vhash *print_funcs, *free_funcs, *copy_funcs;
    /*@=shadow@*/
    static char kbuf[30];
    char *key;

    if (v_debug_flags && v_memory_track)
        v_memchange(V_MEM_FREE, ptr, NULL, 0);

    UPDATE_TABLE(free_funcs, ptr);
    UPDATE_TABLE(print_funcs, ptr);
    UPDATE_TABLE(copy_funcs, ptr);

    free(ptr);
}

/*!
  @brief   Allocate some memory.
  @ingroup util_memory
  @param   size No. of bytes to allocate.
  @return  Allocated memory.
  @see     v_memerr_hook()
*/
void *
v_malloc(size_t size)
{
    void *ptr;

    v_memory_allocated = 1;

    if (size == 0) {
        if (memerr_func == NULL)
            v_fatal("v_malloc(): requested %ld bytes", size);
        else
            memerr_func(size);
    }

    if ((ptr = malloc(size)) == NULL) {
        if (memerr_func == NULL)
            v_fatal("v_malloc(): out of memory (requested %ld bytes)", size);
        else
            memerr_func(size);
    }

    if (v_debug_flags && v_memory_track)
        v_memchange(V_MEM_MALLOC, ptr, NULL, size);

    return ptr;
}

/*!
  @brief   Set hook function for memory errors.
  @ingroup util_memory
  @param   func Function.

  This function is called with the number of bytes requested.  It should
  do something which doesn't involve allocating any more memory, and then
  exit.
*/
void
v_memerr_hook(void (*func)(size_t size))
{
    memerr_func = func;
}

/*!
  @brief   Reallocate memory.
  @ingroup util_memory
  @param   ptr Pointer to reallocate.
  @param   size Size in bytes.
  @return  New memory.
*/
void *
v_realloc(void *ptr, size_t size)
{
    void *newptr = realloc(ptr, size);

    if (size == 0) {
        if (memerr_func == NULL)
            v_fatal("v_realloc(): requested %ld bytes", size);
        else
            memerr_func(size);
    }

    if (newptr == NULL) {
        if (memerr_func == NULL)
            v_fatal("v_realloc(): out of memory (requested %ld bytes)", size);
        else
            memerr_func(size);
    }

    if (v_debug_flags && v_memory_track)
        v_memchange(V_MEM_REALLOC, newptr, ptr, size);

    return newptr;
}

/*!
  @brief   Return a copy of a string.
  @ingroup util_memory
  @param   str String.
  @return  Copy.
*/
char *
v_strdup(const char *str)
{
    char *s;

    if (str == NULL)
        return NULL;

    s = V_ALLOC(char, strlen(str) + 1);
    strcpy(s, str);

    return s;
}
