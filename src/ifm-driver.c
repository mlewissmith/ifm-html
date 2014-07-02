/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Driver functions */

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
#include "ifm-util.h"
#include "ifm-vars.h"

#include "ifm-ps.h"
#include "ifm-fig.h"
#include "ifm-text.h"
#include "ifm-tk.h"
#include "ifm-raw.h"
#include "ifm-rec.h"
#include "ifm-dot.h"

/* Driver info */
driver drivers[] = {
    { "ps", "PostScript",
      &ps_mapfuncs, NULL, NULL, NULL },

    { "fig", "Fig drawing commands",
      &fig_mapfuncs, NULL, NULL, NULL },

    { "text", "Nicely-formatted ASCII text",
      NULL, &text_itemfuncs, &text_taskfuncs, NULL },

    { "rec", "Recording of game commands",
      NULL, NULL, &rec_taskfuncs, NULL },

    { "dot", "Graph of task dependencies",
      NULL, NULL, &dot_taskfuncs, NULL },

    { "raw", "Raw ASCII text fields",
      &raw_mapfuncs, &raw_itemfuncs, &raw_taskfuncs, NULL },

    { "tk", "Tcl/Tk program commands (tkifm)",
      &tk_mapfuncs, &tk_itemfuncs, &tk_taskfuncs, &tk_errfuncs },

    { NULL, NULL, NULL, NULL, NULL, NULL } /* Terminator */
};

/* Macros for setting variables */
#define SET_BOOL(name)     name = var_int(#name)
#define SET_COLOUR(name)   name = var_colour(#name)
#define SET_REAL(name)     name = var_real(#name)
#define SET_FONT(name)     SET_STRING(name)
#define SET_FONTSIZE(name) name = var_real(#name) * font_scale
#define SET_INT(name)      name= var_int(#name)
#define SET_STRING(name)   name = var_string(#name)

/* Control variables */
char *room_text_font, *room_text_colour, *room_colour;
char *room_border_colour, *room_shadow_colour, *item_text_font;
char *item_text_colour, *link_colour, *link_text_colour;
char *link_text_font, *link_updown_string, *link_inout_string;
char *map_title_font, *page_border_colour, *page_background_colour;
char *page_title_font, *page_title_colour, *map_title_colour;
char *room_exit_colour, *page_size, *map_background_colour;
char *map_border_colour;

float room_text_fontsize, room_border_width, room_shadow_xoff;
float room_shadow_yoff, room_exit_width, item_text_fontsize;
float link_arrow_size, link_text_fontsize, link_line_width;
float map_title_fontsize, page_title_fontsize, font_scale, page_margin;
float room_width, room_height, room_size, page_width, page_height;

int room_border_dashed, show_items, link_dashed, show_page_border;
int show_page_title, show_tags, link_spline, show_map_border, show_map_title;

/* Internal functions */
static int itemsort(vscalar **ip1, vscalar **ip2);

/* Set map style variables */
void
set_map_vars(void)
{
    SET_BOOL(show_page_border);
    SET_BOOL(show_page_title);

    SET_BOOL(show_map_border);
    SET_BOOL(show_map_title);

    SET_BOOL(show_tags);

    SET_COLOUR(map_background_colour);
    SET_COLOUR(map_border_colour);
    SET_COLOUR(map_title_colour);

    SET_COLOUR(page_background_colour);
    SET_COLOUR(page_border_colour);
    SET_COLOUR(page_title_colour);

    SET_FONT(map_title_font);
    SET_FONT(page_title_font);

    SET_REAL(font_scale);
    font_scale = V_MAX(font_scale, 0.1);

    SET_REAL(page_margin);

    SET_FONTSIZE(map_title_fontsize);
    SET_FONTSIZE(page_title_fontsize);

    SET_STRING(page_size);
    if (!get_papersize(page_size, &page_width, &page_height))
        fatal("invalid paper size: %s", page_size);

    if (VAR_DEF("page_width"))
        SET_REAL(page_width);

    if (VAR_DEF("page_height"))
        SET_REAL(page_height);

    SET_REAL(room_size);
    room_size = V_MAX(room_size, 0.1);

    SET_REAL(room_width);
    SET_REAL(room_height);
}

/* Set room style variables */
void
set_room_vars(void)
{
    SET_BOOL(room_border_dashed);
    SET_BOOL(show_items);

    SET_COLOUR(item_text_colour);
    SET_COLOUR(room_border_colour);
    SET_COLOUR(room_colour);
    SET_COLOUR(room_exit_colour);
    SET_COLOUR(room_shadow_colour);
    SET_COLOUR(room_text_colour);

    SET_FONT(item_text_font);
    SET_FONT(room_text_font);

    SET_FONTSIZE(item_text_fontsize);
    SET_FONTSIZE(room_text_fontsize);

    SET_REAL(room_border_width);
    SET_REAL(room_exit_width);
    SET_REAL(room_shadow_xoff);
    SET_REAL(room_shadow_yoff);
}

/* Set link style variables */
void
set_link_vars(void)
{
    SET_BOOL(link_dashed);
    SET_BOOL(link_spline);

    SET_COLOUR(link_colour);
    SET_COLOUR(link_text_colour);

    SET_FONT(link_text_font);

    SET_FONTSIZE(link_text_fontsize);

    SET_REAL(link_arrow_size);
    SET_REAL(link_line_width);

    SET_STRING(link_inout_string);
    SET_STRING(link_updown_string);
}

/* Print the map */
void
print_map(int dnum, vlist *sections)
{
    driver drv = drivers[dnum];
    mapfuncs *func = drv.mfunc;

    vhash *sect, *room, *link, *join;
    vlist *sects, *list;
    int num = 1;
    viter i, j;

    if (func == NULL)
        fatal("no map driver for %s output", drv.name);

    sects = vh_pget(map, "SECTS");
    v_iterate(sects, i) {    
        sect = vl_iter_pval(i);
        if (sections != NULL && !vl_iget(sections, num++))
            vh_istore(sect, "NOPRINT", 1);
    }

    set_map_vars();

    if (func->map_start != NULL)
        func->map_start();

    v_iterate(sects, i) {
        sect = vl_iter_pval(i);

        if (vh_iget(sect, "NOPRINT"))
            continue;

        if (func->map_section != NULL)
            func->map_section(sect);

        if (func->map_room != NULL) {
            list = vh_pget(sect, "ROOMS");
            v_iterate(list, j) {
                room = vl_iter_pval(j);
                set_style_list(vh_pget(room, "STYLE"));
                set_room_vars();
                func->map_room(room);
            }
        }

        if (func->map_link != NULL) {
            list = vh_pget(sect, "LINKS");
            v_iterate(list, j) {
                link = vl_iter_pval(j);

                if (vh_iget(link, "HIDDEN"))
                    continue;

                if (vh_iget(link, "NOLINK"))
                    continue;

                set_style_list(vh_pget(link, "STYLE"));
                set_link_vars();
                func->map_link(link);
            }
        }

        if (func->map_endsection != NULL)
            func->map_endsection();
    }

    if (func->map_finish != NULL)
        func->map_finish();

    if (func->map_join != NULL) {
        v_iterate(joins, i) {
            join = vl_iter_pval(i);
            if (!vh_iget(join, "HIDDEN")) {
                set_style_list(vh_pget(join, "STYLE"));
                func->map_join(join);
            }
        }
    }
}

/* Print items table */
void
print_items(int dnum)
{
    driver drv = drivers[dnum];
    itemfuncs *func = drv.ifunc;
    vlist *items;
    vhash *item;
    viter iter;

    items = vh_pget(map, "ITEMS");

    if (func == NULL)
        fatal("no item driver for %s output", drv.name);

    if (func->item_start != NULL)
        func->item_start();

    if (func->item_entry != NULL) {
        vl_sort(items, itemsort);
        v_iterate(items, iter) {
            item = vl_iter_pval(iter);
            set_style_list(vh_pget(item, "STYLE"));
            func->item_entry(item);
        }
    }

    if (func->item_finish != NULL)
        func->item_finish();
}

/* Print task table */
void
print_tasks(int dnum)
{
    driver drv = drivers[dnum];
    taskfuncs *func = drv.tfunc;
    vlist *tasks;
    vhash *task;
    viter iter;

    tasks = vh_pget(map, "TASKS");

    if (func == NULL)
        fatal("no task driver for %s output", drv.name);

    if (func->task_start != NULL)
        func->task_start();

    if (func->task_entry != NULL) {
        v_iterate(tasks, iter) {
            task = vl_iter_pval(iter);
            set_style_list(vh_pget(task, "STYLE"));
            func->task_entry(task);
        }
    }

    if (func->task_finish != NULL)
        func->task_finish();
}

/* Item sort function */
static int
itemsort(vscalar **ip1, vscalar **ip2)
{
    vhash *i1 = vs_pget(*ip1);
    vhash *i2 = vs_pget(*ip2);
    vhash *ir1, *ir2;
    int cmp;

    /* First, by item name */
    cmp = strcmp(vh_sgetref(i1, "DESC"), vh_sgetref(i2, "DESC"));
    if (cmp) return cmp;

    /* Next, by room name */
    ir1 = vh_pget(i1, "ROOM");
    ir2 = vh_pget(i2, "ROOM");
    return strcmp((ir1 == NULL ? "" : vh_sgetref(ir1, "DESC")),
                  (ir2 == NULL ? "" : vh_sgetref(ir2, "DESC")));
}
