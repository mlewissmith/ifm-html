/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup print Printing functions
  @ingroup methods

  These functions print objects in a readable format.
*/

/*!
  @defgroup print_high High-level functions
  @ingroup print

  These functions are for high-level use.
*/

/*!
  @defgroup print_low Low-level functions
  @ingroup print

  These functions are for use when defining new object types (see @ref
  extend).
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "vars-config.h"
#include "vars-hash.h"

static int depth = 0;           /* Recursion depth */
static int indent = 4;          /* Indentation per level */
static int level = 0;           /* Indentation level */
static int printaddr = 1;       /* Whether to print addresses */
static vhash *visited;          /* Hash of visited pointers */

/* Pointer printing functions */
/*@-exportheadervar@*/ 
vhash *print_funcs = NULL; 
/*@=exportheadervar@*/ 

/* Default pointer printing function */
static void (*printfunc)(void *p, FILE *fp) = NULL;

/*!
  @brief   Print indentation according to level.
  @ingroup print_low
  @param   fp Stream.
*/
void
v_indent(FILE *fp)
{
    int num = level * indent;

    while (num >= 8) {
        fputc('\t', fp);
        num -= 8;
    }

    while (num-- > 0)
        fputc(' ', fp);
}

/*!
  @brief   Set indentation width.
  @ingroup print_low
  @param   num Width.
*/
void
v_indent_width(int num)
{
    indent = num;
}

/*!
  @brief   Pop print indentation level.
  @ingroup print_low
*/
void
v_pop_indent(void)
{
    level--;
}

/*!
  @brief   Print contents of a pointer.
  @ingroup print_high
  @param   ptr Pointer to Vars object.
  @param   fp Stream.
*/
void
v_print(void *ptr, FILE *fp)
{
    void (*func)(void *ptr, FILE *fp);
    char *key, buf[V_HEXSTRING_SIZE];
    vtype *type;

    v_print_start();

    if (ptr != NULL)
        key = vh_pkey_buf(ptr, buf);

    if (ptr == NULL) {
        fprintf(fp, "NULL\n");
    } else if (vh_exists(visited, key)) {
        fprintf(fp, "REUSED ADDRESS");

        if (printaddr)
            fprintf(fp, " (%s)", v_hexstring(ptr, NULL));

        fprintf(fp, "\n");
    } else {
        vh_istore(visited, key, 1);
        type = v_type(ptr);

        if (type != NULL && !v_unknown(ptr)) {
            if (type->print != NULL)
                type->print(ptr, fp);
            else
                v_print_type(type, ptr, fp);
        } else if (print_funcs != NULL) {
            /*@-type@*/ 
            if ((func = vh_pget(print_funcs, key)) != NULL)
                func(ptr, fp);
            /*@=type@*/ 
        } else if (printfunc != NULL) {
            printfunc(ptr, fp);
        } else {
            fprintf(fp, "UNKNOWN");

            if (printaddr)
                fprintf(fp, " (%s)", v_hexstring(ptr, NULL));

            fprintf(fp, "\n");
        }
    }

    v_print_finish();
}

/*!
  @brief   Set whether to print memory addresses.
  @ingroup print_low
  @param   flag Yes or no.
*/
void
v_print_address(int flag)
{
    printaddr = flag;
}

/*!
  @brief   Set default pointer printing function.
  @ingroup print_low
  @param   func Function.
*/
void
v_print_default(void (*func)(void *ptr, FILE *fp))
{
    printfunc = func;
}

/*!
  @brief   Finish printing.
  @ingroup print_low
*/
void
v_print_finish(void)
{
    if (--depth == 0)
        vh_destroy(visited);
}

/*!
  @brief   Start printing.
  @ingroup print_low
*/
void
v_print_start(void)
{
    if (depth++ == 0)
        visited = vh_create();
}

/*!
  @brief   Print a type header.
  @ingroup print_low
  @param   type The type variable.
  @param   ptr Pointer being printed.
  @param   fp Stream.
*/
void
v_print_type(vtype *type, void *ptr, FILE *fp)
{
    fprintf(fp, "%s", type->name);

    if (printaddr)
        fprintf(fp, " (%s)", v_hexstring(ptr, NULL));

    fprintf(fp, "\n");
}

/*!
  @brief   Set specific pointer printing function.
  @ingroup print_low
  @param   ptr Pointer being printed.
  @param   func Function.
*/
void
v_print_with(void *ptr, void (*func)(void *ptr, FILE *fp))
{
    char *key, buf[V_HEXSTRING_SIZE];

    if (print_funcs == NULL)
        print_funcs = vh_create();

    key = vh_pkey_buf(ptr, buf);

    if (func != NULL)
        vh_pstore(print_funcs, key, func);
    else
        vh_delete(print_funcs, key);
}

/*!
  @brief   Push print indentation level.
  @ingroup print_low
*/
void
v_push_indent(void)
{
    level++;
}

/*!
  @brief   Print a Vars object to the terminal.
  @ingroup print_high
  @param   ptr Pointer to Vars object.
*/
void
v_show(void *ptr)
{
    static FILE *tty = NULL;

    if (tty == NULL)
        tty = fopen("/dev/tty", "w");
    v_print(ptr, tty);
}
