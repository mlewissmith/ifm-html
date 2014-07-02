/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup type Type declaration functions

  These functions are used to extend the library; see @ref extend for more
  details.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vars-config.h"
#include "vars-macros.h"

#include "vars-buffer.h"
#include "vars-graph.h"
#include "vars-hash.h"
#include "vars-list.h"
#include "vars-queue.h"
#include "vars-scalar.h"

/* Vars magic number */
#define MAGIC 0xff00ee11

#define ALLOC(type, num) \
        malloc((size_t) ((num) * sizeof(type)))

#define REALLOC(var, type, num) \
        realloc((void *) var, (size_t) ((num) * sizeof(type)))

/* Grow a list by malloc() or realloc() */
#define GROW_LIST(type, list, size, num, grow, val) do {        \
    if (size == 0) {                                            \
        size = grow;                                            \
        list = ALLOC(type, size);                               \
    } else if (num == size - 1) {                               \
        size += grow;                                           \
        list = REALLOC(list, type, size);                       \
    }                                                           \
    list[num++] = val;                                          \
} while (0)

/* Unknown type */
vtype *vunknown_type = NULL;

/* List of declared types */
static int num_types = 0;
static int max_types = 0;
static vtype **type_list = NULL;

/*!
  @brief   Create a new object type.
  @ingroup type
  @param   name Object name.
  @param   code Short code.
  @return  New type.
*/
vtype *
v_create(char *name, char *code)
{
    vtype *t;

    /* Check name is valid */
    if (name == NULL)
        v_fatal("v_create(): no type name specified");

    if (code == NULL)
        v_fatal("v_create(): no type code specified");

    if (v_find(code) != NULL)
        v_fatal("v_create(): type %s already exists with code %s",
                name, code);

    /* Create new type */
    t = ALLOC(vtype, 1);

    t->name = strdup(name);
    t->code = strdup(code);

    t->create = NULL;
    t->copy = NULL;
    t->read = NULL;
    t->write = NULL;
    t->freeze = NULL;
    t->thaw = NULL;
    t->print = NULL;
    t->traverse = NULL;
    t->destroy = NULL;
    t->xmldump = NULL;
    t->yamldump = NULL;

    /* Add it to the type list */
    GROW_LIST(vtype *, type_list, max_types, num_types, 10, t);

    return t;
}

/*!
  @brief   Create a new object with a given type code.
  @ingroup type
*/
void *
v_create_object(char *code)
{
    vtype *t;

    v_declare();

    if ((t = v_find(code)) == NULL)
        v_fatal("v_create_object(): unknown type code: %s", code);
    else if (t->create == NULL)
        v_fatal("v_create_object(): type code '%s' has no creation function",
                code);

    return (*t->create)();
}

/*!
  @brief   Declare all the standard types.
  @ingroup type
*/
void
v_declare(void)
{
    static int flag = 0;

    if (!flag++) {
        vb_declare();
        vg_declare();
        vh_declare();
        vl_declare();
        vq_declare();
        vs_declare();
    }
}

/* Return a type given its code */
vtype *
v_find(char *code)
{
    int i;

    for (i = 0; i < num_types; i++)
        if (V_STREQ(code, type_list[i]->code))
            return type_list[i];

    return NULL;
}

/* Return a type given its name */
vtype *
v_find_name(char *name)
{
    int i;

    for (i = 0; i < num_types; i++)
        if (V_STREQ(name, type_list[i]->name))
            return type_list[i];

    return NULL;
}

/*!
  @brief   Return an initialised header for a type.
  @ingroup type
  @param   t Type.
  @return  Header.
*/
vheader *
v_header(vtype *t)
{
    vheader *hdr = ALLOC(vheader, 1);

    hdr->magic = MAGIC;
    hdr->type = t;

    return hdr;
}

/* Return the name of a type */
char *
v_name(vtype *t)
{
    return (t != NULL ? t->name : "NULL");
}

/*!
  @brief   Return the name of an unknown pointer.
  @ingroup type
  @param   ptr Pointer.
  @return  Type name.
*/
char *
v_ptrname(void *ptr)
{
    vheader *id;

    if (ptr == NULL)
	return "NULL";

    id = (vheader *) ptr;
    if (id->magic != MAGIC)
        return "UNKNOWN";

    if (id->type != NULL)
        return id->type->name;

    return "WEIRD";
}

/*!
  @brief   Return the type of an unknown pointer.
  @ingroup type
  @param   ptr Pointer.
  @return  Type.
  @retval  NULL if pointer is \c NULL.
*/
vtype *
v_type(void *ptr)
{
    vheader *id;

    if (vunknown_type == NULL)
        vunknown_type = v_create("UNKNOWN", "U");

    if (ptr == NULL)
	return NULL;

    id = (vheader *) ptr;
    if (id->magic != MAGIC)
        return vunknown_type;

    return id->type;
}

/* Return whether a pointer is of unknown type */
int
v_unknown(void *ptr)
{
    return (ptr != NULL && v_type(ptr) == vunknown_type);
}

/*!
  @brief   Set copy function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_copy_func(vtype *t, void *(*func)())
{
    t->copy = func;
}

/*!
  @brief   Set creation function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_create_func(vtype *t, void *(*func)())
{
    t->create = func;
}

/*!
  @brief   Set destruction function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_destroy_func(vtype *t, void (*func)())
{
    t->destroy = func;
}

/*!
  @brief   Set freeze function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_freeze_func(vtype *t, int (*func)())
{
    t->freeze = func;
}

/*!
  @brief   Set iteration function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_next_func(vtype *t, int (*func)())
{
    t->next = func;
}

/*!
  @brief   Set print function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_print_func(vtype *t, void (*func)())
{
    t->print = func;
}

/*!
  @brief   Set read function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_read_func(vtype *t, void *(*func)())
{
    t->read = func;
}

/*!
  @brief   Set thaw function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_thaw_func(vtype *t, void *(*func)())
{
    t->thaw = func;
}

/*!
  @brief   Set traversal function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_traverse_func(vtype *t, int (*func)())
{
    t->traverse = func;
}

/*!
  @brief   Set write function for an object type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_write_func(vtype *t, int (*func)())
{
    t->write = func;
}

/*!
  @brief   Set XML-dump function for a variable type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_xmldump_func(vtype *t, int (*func)())
{
    t->xmldump = func;
}

/*!
  @brief   Set YAML-dump function for a variable type.
  @ingroup type
  @param   t Type.
  @param   func Function.
*/
void
v_yamldump_func(vtype *t, int (*func)())
{
    t->yamldump = func;
}
