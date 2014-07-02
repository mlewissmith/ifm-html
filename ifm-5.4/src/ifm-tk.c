/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Tcl/Tk output driver */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <vars.h>

#include "ifm-driver.h"
#include "ifm-map.h"
#include "ifm-task.h"
#include "ifm-util.h"
#include "ifm-vars.h"
#include "ifm-tk.h"
#include "ifm-text.h"

#define PRINT_COLOUR(name) \
        if (var_changed(#name)) \
                printf("set ifm(%s) {%s}\n", #name, var_string(#name))

#define PRINT_FONTDEF(name) \
        if (var_changed(#name)) \
                printf("set ifm(%s) {%s}\n", #name, var_string(#name))

#define PRINT_INT(name) \
        if (var_changed(#name)) \
                printf("set ifm(%s) %d\n", #name, var_int(#name))

#define PRINT_REAL(name) \
        if (var_changed(#name)) \
                printf("set ifm(%s) %g\n", #name, var_real(#name))

#define PRINT_STRING(name) \
        if (var_changed(#name)) \
                printf("set ifm(%s) {%s}\n", #name, var_string(#name))

#define PRINT_BOOL(name) \
        if (var_changed(#name)) \
                printf("set ifm(%s) %s\n", #name, var_int(#name) ? "true" : "false")

/* Map function list */
mapfuncs tk_mapfuncs = {
    tk_map_start,
    tk_map_section,
    tk_map_room,
    tk_map_link,
    NULL,
    NULL,
    NULL
};

/* Item function list */
itemfuncs tk_itemfuncs = {
    tk_item_start,
    tk_item_entry,
    tk_item_finish
};

/* Task function list */
taskfuncs tk_taskfuncs = {
    tk_task_start,
    tk_task_entry,
    tk_task_finish
};

/* Error function list */
errfuncs tk_errfuncs = {
    tk_warning,
    tk_error
};

/* Internal functions */
static void tk_print_room_vars(void);
static void tk_print_link_vars(void);

/* Map functions */
void
tk_map_start(void)
{
    /* Set room names */
    setup_room_names();

    /* Canvas variables */
    PRINT_INT(map_canvas_width);
    PRINT_INT(map_canvas_height);
    PRINT_COLOUR(map_background_colour);
    PRINT_REAL(room_size);
    PRINT_REAL(room_width);
    PRINT_REAL(room_height);

    /* Room style variables */
    tk_print_room_vars();

    /* Link style variables */
    tk_print_link_vars();
}

void
tk_map_section(vhash *sect)
{
    char *title = vh_sgetref(sect, "TITLE");
    static int num = 0;
    V_BUF_DECL;

    num++;

    if (strlen(title) == 0)
        V_BUF_SET1("Map section %d", num);
    else
        V_BUF_SET(title);

    put_string("AddSect {%s} %d %d\n", V_BUF_VAL,
               vh_iget(sect, "XLEN"),
               vh_iget(sect, "YLEN"));
}

void
tk_map_room(vhash *room)
{
    vlist *items, *ex, *ey;
    char *itemlist = NULL;
    int x, y, xoff, yoff;
    viter iter;

    /* Room style variables */
    tk_print_room_vars();

    /* Build item list if required */
    items = vh_pget(room, "ITEMS");
    if (items != NULL && vl_length(items) > 0) {
        vhash *item;
        vlist *list;

        list = vl_create();
        v_iterate(items, iter) {
            item = vl_iter_pval(iter);
            if (!vh_iget(item, "HIDDEN"))
                vl_spush(list, vh_sgetref(item, "DESC"));
        }

        if (vl_length(list) > 0)
            itemlist = vl_join(list, ", ");

        vl_destroy(list);
    }

    /* Do room command */
    x = vh_iget(room, "X");
    y = vh_iget(room, "Y");
    put_string("AddRoom {%s} {%s} %d %d\n",
               vh_sgetref(room, "RDESC"),
               (itemlist != NULL ? itemlist : ""), x, y);

    /* Do room exit commands (if any) */
    ex = vh_pget(room, "EX");
    ey = vh_pget(room, "EY");
    if (ex != NULL) {
        while (vl_length(ex) > 0) {
            xoff = vl_ishift(ex);
            yoff = vl_ishift(ey);
            printf("AddExit {%d %d} {%d %d}\n", x, x + xoff, y, y + yoff);
        }
    }
}

void
tk_map_link(vhash *link)
{
    vlist *x = vh_pget(link, "X");
    vlist *y = vh_pget(link, "Y");
    int go = vh_iget(link, "GO");
    int updown = (go == D_UP || go == D_DOWN);
    int inout = (go == D_IN || go == D_OUT);

    /* Link style variables */
    tk_print_link_vars();

    printf("AddLink");
    printf(" {%s}", vl_join(x, " "));
    printf(" {%s}", vl_join(y, " "));
    printf(" %d %d %d\n",
           updown, inout,
           vh_iget(link, "ONEWAY"));
}

/* Item functions */
void
tk_item_start(void)
{
    printf("set itemlist {");
}

void
tk_item_entry(vhash *item)
{
    text_item_entry(item);
}

void
tk_item_finish(void)
{
    printf("}\n");
}

/* Task functions */
void
tk_task_start(void)
{
    printf("set tasklist {");
}

void
tk_task_entry(vhash *task)
{
    text_task_entry(task);
}

void
tk_task_finish(void)
{
    text_task_finish();
    printf("}\n");
}

/* Error functions */
void
tk_warning(char *file, int line, char *msg)
{
    if (line > 0) {
        printf("GotoLine %d\n", line);
        printf("Warning {Warning: line %d: %s}\n", line, msg);
    } else {
        printf("Warning {Warning: %s}\n", msg);
    }
}

void
tk_error(char *file, int line, char *msg)
{
    if (line > 0) {
        printf("GotoLine %d\n", line);
        printf("Error {Error: line %d: %s}\n", line, msg);
    } else {
        printf("Error {Error: %s}\n", msg);
    }

    exit(0);
}

/* Print room style variables */
static void
tk_print_room_vars(void)
{
    PRINT_FONTDEF(room_text_fontdef);
    PRINT_COLOUR(room_colour);
    PRINT_COLOUR(room_text_colour);
    PRINT_COLOUR(room_border_colour);
    PRINT_REAL(room_border_width);
    PRINT_REAL(room_shadow_xoff);
    PRINT_REAL(room_shadow_yoff);
    PRINT_COLOUR(room_shadow_colour);
    PRINT_COLOUR(room_exit_colour);
    PRINT_REAL(room_exit_width);
    PRINT_BOOL(show_items);
    PRINT_FONTDEF(item_text_fontdef);
    PRINT_COLOUR(item_text_colour);
}

/* Print link style variables */
static void
tk_print_link_vars(void)
{
    PRINT_COLOUR(link_colour);
    PRINT_BOOL(link_spline);
    PRINT_REAL(link_arrow_size);
    PRINT_COLOUR(link_text_colour);
    PRINT_FONTDEF(link_text_fontdef);
    PRINT_REAL(link_line_width);
    PRINT_STRING(link_updown_string);
    PRINT_STRING(link_inout_string);
}
