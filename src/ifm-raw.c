/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Raw output driver */

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
#include "ifm-raw.h"
#include "ifm-vars.h"
#include "ifm-task.h"

/* Map function list */
mapfuncs raw_mapfuncs = {
    raw_map_start,
    raw_map_section,
    raw_map_room,
    raw_map_link,
    raw_map_join,
    NULL,
    NULL
};

/* Item function list */
itemfuncs raw_itemfuncs = {
    NULL,
    raw_item_entry,
    NULL
};

/* Task function list */
taskfuncs raw_taskfuncs = {
    NULL,
    raw_task_entry,
    NULL
};

/* Map functions */
void
raw_map_start(void)
{
    char *title;

    if (vh_exists(map, "TITLE"))
        title = vh_sgetref(map, "TITLE");
    else
        title = "Interactive Fiction map";

    put_string("title: %s\n", title);
}

void
raw_map_section(vhash *sect)
{
    static int count = 0;
    char buf[100];

    count++;
    if (!vh_exists(sect, "TITLE")) {
        sprintf(buf, "Map section %d", count);
        vh_sstore(sect, "TITLE", buf);
    }

    put_string("\nsection: %s\n", vh_sgetref(sect, "TITLE"));
    printf("width: %d\n", vh_iget(sect, "XLEN"));
    printf("height: %d\n", vh_iget(sect, "YLEN"));
}

void
raw_map_room(vhash *room)
{
    vlist *ex, *ey;

    vlist *notes = vh_pget(room, "NOTE");
    viter iter;

    printf("\nroom: %d\n", vh_iget(room, "ID"));
    put_string("name: %s\n", vh_sgetref(room, "DESC"));

    printf("rpos: %d %d\n", vh_iget(room, "X"), vh_iget(room, "Y"));

    ex = vh_pget(room, "EX");
    ey = vh_pget(room, "EY");

    if (ex != NULL && ey != NULL)
        while (vl_length(ex) > 0 && vl_length(ey) > 0)
            printf("exit: %d %d\n", vl_ishift(ex), vl_ishift(ey));

    if (notes != NULL) {
        v_iterate(notes, iter)
            put_string("note: %s\n", vl_iter_svalref(iter));
    }

}

void
raw_map_link(vhash *link)
{
    vlist *x, *y, *cmds;
    vhash *from, *to;
    viter iter;
    int go;

    from = vh_pget(link, "FROM");
    to = vh_pget(link, "TO");
    printf("\nlink: %d %d\n", vh_iget(from, "ID"), vh_iget(to, "ID"));

    x = vh_pget(link, "X");
    y = vh_pget(link, "Y");
    while (vl_length(x) > 0 && vl_length(y) > 0)
        printf("lpos: %d %d\n", vl_ishift(x), vl_ishift(y));

    if (vh_iget(link, "ONEWAY"))
        printf("oneway: 1\n");

    go = vh_iget(link, "GO");
    if (go != D_NONE)
        printf("go: %s\n", dirinfo[go].sname);

    if ((cmds = vh_pget(link, "CMD")) != NULL) {
        v_iterate(cmds, iter)
            put_string("cmd: %s\n", vl_iter_svalref(iter));
    }
}

void
raw_map_join(vhash *join)
{
    vhash *from, *to;
    vlist *cmds;
    viter iter;
    int go;

    from = vh_pget(join, "FROM");
    to = vh_pget(join, "TO");
    printf("\njoin: %d %d\n", vh_iget(from, "ID"), vh_iget(to, "ID"));

    if (vh_iget(join, "ONEWAY"))
        printf("oneway: 1\n");

    go = vh_iget(join, "GO");
    if (go != D_NONE)
        printf("go: %s\n", dirinfo[go].sname);

    if ((cmds = vh_pget(join, "CMD")) != NULL) {
        v_iterate(cmds, iter)
            put_string("cmd: %s\n", vl_iter_svalref(iter));
    }
}

/* Item functions */
void
raw_item_entry(vhash *item)
{
    vlist *notes = vh_pget(item, "NOTE");
    vhash *room = vh_pget(item, "ROOM");
    int score = vh_iget(item, "SCORE");
    vhash *task, *reach;
    vlist *list;
    viter iter;

    printf("\nitem: %d\n", vh_iget(item, "ID"));
    put_string("name: %s\n", vh_sgetref(item, "DESC"));

    if (vh_exists(item, "TAG"))
        printf("tag: %s\n", vh_sgetref(item, "TAG"));

    if (room != NULL)
        printf("room: %d\n", vh_iget(room, "ID"));

    if (score > 0)
        printf("score: %d\n", score);

    if (vh_exists(item, "LEAVE"))
        printf("leave: 1\n");

    if (vh_iget(item, "HIDDEN"))
        printf("hidden: 1\n");

    if (vh_iget(item, "FINISH"))
        printf("finish: 1\n");

    if ((list = vh_pget(item, "RTASKS")) != NULL) {
        v_iterate(list, iter) {
            task = vl_iter_pval(iter);
            printf("after: %d\n", vh_iget(task, "ID"));
        }
    }

    if ((list = vh_pget(item, "TASKS")) != NULL) {
        v_iterate(list, iter) {
            task = vl_iter_pval(iter);
            printf("needed: %d\n", vh_iget(task, "ID"));
        }
    }

    if ((list = vh_pget(item, "NROOMS")) != NULL) {
        v_iterate(list, iter) {
            room = vl_iter_pval(iter);
            printf("enter: %d\n", vh_iget(room, "ID"));
        }
    }

    if ((list = vh_pget(item, "NLINKS")) != NULL) {
        v_iterate(list, iter) {
            reach = vl_iter_pval(iter);
            room = vh_pget(reach, "FROM");
            printf("move: %d", vh_iget(room, "ID"));
            room = vh_pget(reach, "TO");
            printf(" %d\n", vh_iget(room, "ID"));
        }
    }

    if (notes != NULL) {
        v_iterate(notes, iter)
            put_string("note: %s\n", vl_iter_svalref(iter));
    }
}

/* Task functions */
void
raw_task_entry(vhash *task)
{
    vlist *notes = vh_pget(task, "NOTE");
    vhash *room = vh_pget(task, "ROOM");
    vlist *cmds = vh_pget(task, "CMD");
    vhash *item;
    vlist *itemlist;
    
    int score = vh_iget(task, "SCORE");
    viter iter;
    int type;

    type = vh_iget(task, "TYPE");

    printf("\ntask: %d\n", vh_iget(task, "ID"));

    switch (type) {
    case T_MOVE:
      printf("type: MOVE\n");
      break;
    case T_GET:
      printf("type: GET\n");
      if ((item  = vh_pget(task, "DATA")) != NULL)
          printf("get: %d\n", vh_iget(item, "ID"));
      break;
    case T_DROP:
      printf("type: DROP\n");
      break;
    case T_GOTO:
      printf("type: GOTO\n");
      break;
    case T_USER:
      printf("type: USER\n");
      if ((itemlist = vh_pget(task, "GIVE")) != NULL) {
          v_iterate(itemlist, iter) {
              item = vl_iter_pval(iter);
              printf("give: %d\n", vh_iget(item, "ID"));
          }
      }
      break;
    }

    put_string("name: %s\n", vh_sgetref(task, "DESC"));

    if (vh_exists(task, "TAG"))
        printf("tag: %s\n", vh_sgetref(task, "TAG"));

    if (room != NULL)
        printf("room: %d\n", vh_iget(room, "ID"));

    if (cmds != NULL) {
        v_iterate(cmds, iter)
            put_string("cmd: %s\n", vl_iter_svalref(iter));
    }

    if (score > 0)
        printf("score: %d\n", score);

    if (notes != NULL) {
        v_iterate(notes, iter)
            put_string("note: %s\n", vl_iter_svalref(iter));
    }
}
