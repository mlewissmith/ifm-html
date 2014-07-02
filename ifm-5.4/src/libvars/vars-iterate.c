/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup iterate Iteration functions
  @ingroup methods

  Some Vars object types allow @e iteration -- that is, sequential access
  of their contents.  There are several functions and macros that implement
  iteration, as described below.

  To keep track of the current iteration position in a Vars object, an @e
  iterator is used.  This is an object of type @ref viter.  Before
  iteration, it must be initialized with the object to be iterated over,
  using v_iter().  Note that @ref viter is declared as a structure, not a
  pointer to one.

  Then, to iterate over the object, you call v_next().  Each call moves to
  the next entry in the object, and returns whether there are any more
  entries left.  Information on the current entry is found in the @ref
  viter object, which can be accessed using object-specific macros; see the
  object-specific iteration functions (vl_next(), vh_next(), etc) for the
  macro names.

  As a convenience, there's a v_iterate() macro which initializes a @ref
  viter object using v_iter() and sets up a while loop using v_next().
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vars-config.h"
#include "vars-iterate.h"
#include "vars-macros.h"
#include "vars-memory.h"
#include "vars-type.h"

/*!
  @brief   Initialize iteration over an object.
  @ingroup iterate
  @param   object Object to be iterated over.
  @param   iter Iterator reference.
  @see     v_iterate()
*/
void
v_iter(void *object, viter *iter)
{
    V_ZERO(iter, 1);
    iter->object = object;
    iter->count = -1;
}

/*!
  @brief   Return whether an object type is iterable.
  @ingroup iterate
  @param   t Object type.
  @return  Whether iterable.
*/
int
v_iterable(vtype *t)
{
    return (t->next != NULL);
}

/*!
  @brief   Iterate on an iterator.
  @ingroup iterate
  @param   iter Iterator reference.
  @return  Whether to continue.
  @see     v_iterate()

  This function looks up the type of the iterator object and calls the
  appropriate iteration function (if any).
*/
int
v_next(viter *iter)
{
    vtype *t;

    if ((t = v_type(iter->object)) == NULL)
        return 0;

    if (t->next == NULL)
        return 0;

    iter->count++;
    return t->next(iter);
}
