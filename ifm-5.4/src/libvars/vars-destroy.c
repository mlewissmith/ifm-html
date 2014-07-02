/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup destroy Destruction functions
  @ingroup methods

  In all cases, whenever a function returns a newly-created Vars object to
  you, you are responsible for deallocating it.  There are several ways to
  do this.  Firstly, the standard destruction functions (vs_destroy(),
  vl_destroy(), etc.) can be used to destroy individual objects and their
  contents.

  However, if an object contains a pointer to another object, this other
  object will not be touched.  For those times when you have created a
  complicated data structure (e.g. a hash containing pointers to other
  hashes or lists, and so on) and you want to destroy all of it, these
  functions are available.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vars-config.h"
#include "vars-hash.h"

/* List of pointers to destroy */
static vlist *pointers = NULL; 

/* Pointer destruction functions */
/*@-exportheadervar@*/ 
vhash *free_funcs = NULL; 
/*@=exportheadervar@*/ 

/* Default destruction function */
static void (*freefunc)(void *p) = NULL;

/* Internal functions */
static int collect_pointers(void *ptr);

/* Pointer collection function */
static int
collect_pointers(void *ptr)
{
    if (!v_traverse_seen(ptr))
        vl_ppush(pointers, ptr);

    return 0;
}

/*!
  @brief   Recursively destroy a Vars object.
  @ingroup destroy
  @param   ptr Pointer to Vars object.

  Given an anonymous pointer to data, recursively destroy it and all its
  pointer references.  This is done by firstly traversing the pointer tree
  using v_traverse() and collecting a list of pointers which require
  destruction.  Then each pointer is destroyed depending on its type.  If
  the pointer type is unknown, because you have explicitly added it
  yourself, then by default nothing is done.
*/
void
v_destroy(void *ptr)
{
    char *key, buf[V_HEXSTRING_SIZE];
    static int destroying = 0;
    void (*func)(void *ptr);
    vtype *type;
    viter iter;
    void *p;

    /* Check for recursive calling */
    if (destroying++)
        return;

    /* Traverse data and collect pointers */
    pointers = vl_create();
    v_traverse(ptr, collect_pointers);

    /* Destroy each one */
    v_iterate(pointers, iter) {
        p = vl_iter_pval(iter);
        type = v_type(p);

        if (type != NULL && type->destroy != NULL) {
            type->destroy(p);
        } else {
            key = vh_pkey_buf(p, buf);

            /*@-type@*/ 
            if (free_funcs != NULL &&
                (func = vh_pget(free_funcs, key)) != NULL) {
                /* Use explicit destruction function */
                func(p);
                vh_delete(free_funcs, key);
            } else if (freefunc != NULL) {
                /* Use default function */
                freefunc(p);
            }
            /*@=type@*/ 
        }
    }

    /* Clean up */
    vl_destroy(pointers);
    destroying = 0;
}

/*!
  @brief   Set a default destruction function.
  @ingroup destroy
  @param   func Function (or \c NULL to unset it).

  Set a default destruction function for any anonymous pointers that don't
  have an explicit destruction function associated.  Using a \c NULL
  argument will unset this default.  Note that if this is set, then no
  unknown pointer will be left untouched by v_destroy().
*/
void
v_destroy_default(void (*func)(void *ptr))
{
    freefunc = func;
}

/*!
  @brief   Set a destruction function for a pointer.
  @ingroup destroy
  @param   ptr Pointer to Vars object.
  @param   func Function (or \c NULL to unset it).

  Given an anonymous pointer to data, associate with it a function that can
  be used to destroy it.  Then, when v_destroy() sees it, it will use that
  function.  Note that when the pointer is destroyed via this function, the
  association is lost.

  @note This function will have no effect on pointers to standard Vars
  objects.
*/
void
v_destroy_with(void *ptr, void (*func)(void *ptr))
{
    char buf[V_HEXSTRING_SIZE];

    if (free_funcs == NULL)
        free_funcs = vh_create();

    vh_pstore(free_funcs, vh_pkey_buf(ptr, buf), func);
}
