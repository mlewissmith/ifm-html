/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup traverse Traversal functions
  @ingroup methods

  Several types of object may contain pointers to other objects.  This can
  lead to a data structure nested over many levels.  In some situations you
  need to apply a function to the whole data structure -- this involves
  'traversing' the data.  There are several functions available for doing
  this.
*/

/*!
  @defgroup traverse_high High-level functions
  @ingroup traverse

  These functions are for high-level use.
*/

/*!
  @defgroup traverse_low Low-level functions
  @ingroup traverse

  These functions are for use when defining new object types (see @ref
  extend).
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vars-hash.h"

#define PUSH if (depth++ == 0) vh_init(visited)
#define POP  depth--

#define VISIT(ptr)                                                      \
        vh_istore(visited, vh_pkey_buf(ptr, buf), 1)

#define VISITED(ptr)                                                    \
        (depth > 0 && vh_exists(visited, vh_pkey_buf(ptr, buf)))

/* Traversal depth */
static int depth = 0;

/* Hash of visited pointers */
static vhash *visited = NULL; 

/*!
  @brief   Pop traversal depth.
  @ingroup traverse_low
*/
void
v_pop_traverse(void)
{
    POP;
}

/*!
  @brief   Push traversal depth.
  @ingroup traverse_low
*/
void
v_push_traverse(void *ptr)
{
    char buf[V_HEXSTRING_SIZE];
    PUSH;
    VISIT(ptr);
}

/*!
  @brief   Traverse a pointer tree.
  @ingroup traverse_high
  @param   ptr Pointer to Vars object.
  @param   func Function to call on each one.
  @return  Whether traversal aborted.

  This is the generic traversal function.  It traverses the given pointer
  depending on its type.  You must supply a function which takes a pointer
  as argument and which returns an integer.  This function is called on
  each traversed pointer.  If it returns zero, the traversal continues,
  otherwise traversal is aborted and v_traverse() returns the non-zero
  value.  If the pointer is a recognized object which has not been
  traversed before, it is traversed according to its type (see @ref extend
  for more details).
*/
int
v_traverse(void *ptr, int (*func)(void *ptr))
{
    char buf[V_HEXSTRING_SIZE];
    int flag = 0;
    vtype *type;

    if (ptr == NULL)
        return 0;

    if ((type = v_type(ptr)) == NULL || type->traverse == NULL) {
        if (!VISITED(ptr)) {
            PUSH;
            flag = func(ptr);
            POP;
            VISIT(ptr);
        }
    } else {
        flag = type->traverse(ptr, func);
    }

    return flag;
}

/*!
  @brief   Return whether a pointer has been traversed.
  @ingroup traverse_high
  @param   ptr Pointer to Vars object.
  @return  Yes or no.

  This function is for use inside a traversal function.
*/
int
v_traverse_seen(void *ptr)
{
    char buf[V_HEXSTRING_SIZE];
    return VISITED(ptr);
}
