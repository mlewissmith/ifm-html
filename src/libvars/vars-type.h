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
  @brief Type declaration functions and macros.
  @ingroup type
*/

#ifndef VARS_TYPE_H
#define VARS_TYPE_H

/* Type definition */
struct v_type {
    char *name;                 /* Type name */
    char *code;                 /* Type code */

    void *(*create)();          /* Creation function */
    void *(*copy)();            /* Copy function */
    int (*next)();              /* Iteration function */
    void (*print)();            /* Print function */
    void *(*read)();            /* Read function */
    int (*write)();             /* Write function */
    int (*freeze)();            /* Freeze function */
    void *(*thaw)();            /* Thaw function */
    int (*xmldump)();           /* Dump-XML function */
    int (*yamldump)();          /* Dump-YAML function */
    int (*traverse)();          /* Traversal function */
    void (*destroy)();          /* Destruction function */
};

struct v_header {
    int magic;                  /* Magic number */
    struct v_type *type;        /* Variable type */
};

/*! @brief Variable-type type. */
typedef struct v_type vtype;

/*! @brief Type header type. */
typedef struct v_header vheader;

/* Include auxiliary type stuff */
#include <vars-debug.h>
#include <vars-destroy.h>
#include <vars-freeze.h>
#include <vars-iterate.h>
#include <vars-print.h>
#include <vars-traverse.h>
#include <vars-utils.h>
#include <vars-write.h>
#include <vars-xml.h>

#ifdef __cplusplus
extern "C" {
#endif

extern vtype *v_create(char *name, char *code);
extern void *v_create_object(char *code);
extern void v_declare(void);
extern vtype *v_find(char *code);
extern vtype *v_find_name(char *name);
extern vheader *v_header(vtype *t);
extern char *v_name(vtype *t);
extern char *v_ptrname(void *ptr);
extern vtype *v_type(void *ptr);
extern int v_unknown(void *ptr);

extern void v_copy_func(vtype *t, void *(*func)());
extern void v_create_func(vtype *t, void *(*func)());
extern void v_destroy_func(vtype *t, void (*func)());
extern void v_freeze_func(vtype *t, int (*func)());
extern void v_next_func(vtype *t, int (*func)());
extern void v_print_func(vtype *t, void (*func)());
extern void v_read_func(vtype *t, void *(*func)());
extern void v_thaw_func(vtype *t, void *(*func)());
extern void v_traverse_func(vtype *t, int (*func)());
extern void v_write_func(vtype *t, int (*func)());
extern void v_xmldump_func(vtype *t, int (*func)());
extern void v_yamldump_func(vtype *t, int (*func)());

#ifdef __cplusplus
}
#endif

#endif
