/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup copy Copying functions
  @ingroup methods

  The type-specific copy functions (vl_copy(), vh_copy(), etc.) make a
  "shallow copy" of objects: if any other objects are referenced by them,
  these are not copied, only referenced.  If you want to copy recursively
  (a "deep copy"), use the function v_copy().  This traverses the
  referenced objects and creates new copies, so that the copied data is
  completely independent of the original.

  During copying, if a pointer to something that isn't a Vars object is
  encountered, the default behaviour is to give a fatal error.  You can
  change this for a particular pointer using v_copy_with(), or for all
  pointers using v_copy_default().
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vars-config.h"
#include "vars-copy.h"
#include "vars-hash.h"

/* Deep-copy flag */
int v_deepcopy = 0;

/* Pointer copy functions */
/*@-exportheadervar@*/ 
vhash *copy_funcs = NULL; 
/*@=exportheadervar@*/ 

/* Default copy function */
static void *(*copyfunc)(void *p) = NULL;

/*!
  @brief   Deep-copy an object.
  @ingroup copy
  @param   ptr Vars object.
  @return  Copy of the object (or \c NULL if the pointer is \c NULL).

  Make a "deep copy" of an object by recursively copying all referenced
  pointers inside the object.
*/
void *
v_copy(void *ptr)
{
    void (*pfunc)(void *ptr, FILE *fp) = NULL;
    extern vhash *print_funcs, *free_funcs;
    char *key, buf[V_HEXSTRING_SIZE];
    void *(*cfunc)(void *ptr) = NULL;
    void (*dfunc)(void *ptr) = NULL;
    static vhash *seen = NULL;
    static int depth = 0;
    void *cptr;
    vtype *t;

    /* Do nothing if NULL */
    if (ptr == NULL)
        return NULL;

    /* Initialise if required */
    if (depth++ == 0) {
        v_deepcopy = 1;
        if (seen == NULL)
            seen = vh_create_size(1000);
    }

    key = vh_pkey_buf(ptr, buf);

    if (print_funcs != NULL)
        pfunc = vh_pget(print_funcs, key);

    if (free_funcs != NULL)
        dfunc = vh_pget(free_funcs, key);

    if ((cptr = vh_pget(seen, key)) == NULL) {
        /* Copy the pointer */
        if ((t = v_type(ptr)) != NULL && t->copy != NULL) {
            /* Standard function */
            ptr = t->copy(ptr);
        } else if (copy_funcs != NULL &&
                   (cfunc = vh_pget(copy_funcs, key)) != NULL) {
            /* Specific copy function */
            ptr = cfunc(ptr);
        } else if (copyfunc != NULL) {
            /* Default copy function */
            ptr = copyfunc(ptr);
        } else {
            v_fatal("v_copy(): no copy function for pointer");
        }

        /* Flag it as copied */
        vh_pstore(seen, key, ptr);
    } else {
        /* Seen before -- point to the copy */
        ptr = cptr;
    }

    if (ptr == NULL)
        v_fatal("v_copy(): copy function failed");

    /* Update pointer-specific functions if required */
    key = vh_pkey_buf(ptr, buf);

    if (cfunc != NULL)
        vh_pstore(copy_funcs, key, cfunc);

    if (pfunc != NULL)
        vh_pstore(print_funcs, key, pfunc);

    if (dfunc != NULL)
        vh_pstore(free_funcs, key, dfunc);

    /* Clean up if required */
    if (--depth == 0) {
        vh_empty(seen);
        v_deepcopy = 0;
    }

    return ptr;
}

/*!
  @brief   Set a default copy function.
  @ingroup copy
  @param   func Function (or \c NULL to unset it).

  Set a default copy function for any anonymous pointers that don't have an
  explicit copy function associated.  Using a \c NULL argument will unset
  this default.
*/
void
v_copy_default(void *(*func)(void *ptr))
{
    copyfunc = func;
}

/*!
  @brief   Set a copy function for a pointer.
  @ingroup copy
  @param   ptr Pointer to Vars object.
  @param   func Function (or \c NULL to unset it).

  Given an anonymous pointer to data, associate with it a function that can
  be used to copy it.  Then, when v_copy() sees it, it will use that
  function.  When copied, the new pointer is also marked to be copied with
  this function.

  @note This function will have no effect on pointers to standard Vars
  objects.
*/
void
v_copy_with(void *ptr, void *(*func)(void *ptr))
{
    char buf[V_HEXSTRING_SIZE];

    if (copy_funcs == NULL)
        copy_funcs = vh_create();

    vh_pstore(copy_funcs, vh_pkey_buf(ptr, buf), func);
}
