/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Text output driver */

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
#include "ifm-text.h"

/* Item function list */
itemfuncs text_itemfuncs = {
    NULL,
    text_item_entry,
    NULL
};

/* Task function list */
taskfuncs text_taskfuncs = {
    NULL,
    text_task_entry,
    text_task_finish,
};

/* Total score */
static int total = 0;

/* Total distance travelled */
static int travel = 0;

/* Item functions */
void
text_item_entry(vhash *item)
{
    vlist *notes = vh_pget(item, "NOTE");
    vhash *room, *task, *reach;
    static int count = 0;
    vlist *list;
    char *title;
    V_BUF_DECL;
    viter iter;

    if (count++ == 0) {
        if (vh_exists(map, "TITLE"))
            title = vh_sgetref(map, "TITLE");
        else
            title = "Interactive Fiction game";
        put_string("Item list for %s\n", title);
    }

    put_string("\n%s:\n", vh_sgetref(item, "DESC"));

    if ((room = vh_pget(item, "ROOM")) == NULL)
        printf("   carried at the start of the game\n");
    else
        printf("   %s in %s\n",
               (vh_iget(item, "HIDDEN") ? "hidden" : "seen"),
               vh_sgetref(room, "DESC"));

    if (vh_exists(item, "SCORE"))
        printf("   scores %d points when picked up\n",
               vh_iget(item, "SCORE"));

    if (vh_exists(item, "LEAVE"))
        printf("   may have to be dropped when moving\n");

    if (vh_exists(item, "FINISH"))
        printf("   finishes the game when picked up\n");

    if ((list = vh_pget(item, "RTASKS")) != NULL) {
        printf("   obtained after:\n");
        v_iterate(list, iter) {
            task = vl_iter_pval(iter);
            if ((room = vh_pget(task, "ROOM")) == NULL)
                V_BUF_SET(vh_sgetref(task, "DESC"));
            else
                V_BUF_SET2("%s (%s)",
                           vh_sgetref(task, "DESC"),
                           vh_sgetref(room, "DESC"));
            put_string("      %s\n", V_BUF_VAL);
        }
    }

    if ((list = vh_pget(item, "TASKS")) != NULL) {
        printf("   needed for:\n");
        v_iterate(list, iter) {
            task = vl_iter_pval(iter);
            if ((room = vh_pget(task, "ROOM")) == NULL)
                V_BUF_SET(vh_sgetref(task, "DESC"));
            else
                V_BUF_SET2("%s (%s)",
                           vh_sgetref(task, "DESC"),
                           vh_sgetref(room, "DESC"));
            put_string("      %s\n", V_BUF_VAL);
        }
    }

    if ((list = vh_pget(item, "NROOMS")) != NULL) {
        printf("   needed to enter:\n");
        v_iterate(list, iter) {
            room = vl_iter_pval(iter);
            put_string("      %s\n", vh_sgetref(room, "DESC"));
        }
    }

    if ((list = vh_pget(item, "NLINKS")) != NULL) {
        printf("   needed to move:\n");
        v_iterate(list, iter) {
            reach = vl_iter_pval(iter);
            room = vh_pget(reach, "FROM");
            put_string("      %s to", vh_sgetref(room, "DESC"));
            room = vh_pget(reach, "TO");
            put_string(" %s\n", vh_sgetref(room, "DESC"));
        }
    }

    if (notes != NULL) {
        v_iterate(notes, iter)
            put_string("   note: %s\n", vl_iter_svalref(iter));
    }
}

/* Task functions */
void
text_task_entry(vhash *task)
{
    vlist *triggers = vh_pget(task, "DO");
    vlist *notes = vh_pget(task, "NOTE");
    vhash *room = vh_pget(task, "ROOM");
    vlist *cmds = vh_pget(task, "CMD");
    static vhash *lastroom = NULL;
    static int moved = 0;
    static int count = 0;
    int type, score;
    vhash *otask;
    char *title;
    viter iter;

    if (count == 0) {
        if (vh_exists(map, "TITLE"))
            title = vh_sgetref(map, "TITLE");
        else
            title = "Interactive Fiction game";
        put_string("Task list for %s\n", title);
    }

    type = vh_iget(task, "TYPE");

    if (type == T_MOVE) {
        if (count == 0)
            put_string("\nStart: %s\n", vh_sgetref(startroom, "DESC"));

        if (!moved)
            printf("\n");

        put_string("%s", vh_sgetref(task, "DESC"));
        if (cmds != NULL)
            put_string(" (%s)", vl_join(cmds, ". "));

        printf("\n");
        travel++;
        moved++;
    } else {
        if (room != NULL && (moved || room != lastroom))
            put_string("\n%s:\n", vh_sgetref(room, "DESC"));
        else if (!count)
            printf("\nFirstly:\n");

        put_string("   %s\n", vh_sgetref(task, "DESC"));

        if (cmds != NULL) {
            if (vl_length(cmds) > 0) {
                v_iterate(cmds, iter)
                    put_string("      cmd: %s\n", vl_iter_svalref(iter));
            } else {
                printf("      no action required\n");
            }
        }

        moved = 0;
    }

    if (triggers != NULL) {
        v_iterate(triggers, iter) {
            otask = vl_iter_pval(iter);

            if (type != T_MOVE)
                printf("   ");

            put_string("   also does: %s\n", vh_sgetref(otask, "DESC"));
        }
    }

    if ((score = vh_iget(task, "SCORE")) > 0) {
        if (type != T_MOVE)
            printf("   ");

        printf("   score: %d\n", score);
    }

    if (notes != NULL) {
        v_iterate(notes, iter) {
            if (type != T_MOVE)
                printf("   ");

            put_string("   note: %s\n", vl_iter_svalref(iter));
        }
    }

    if (room != NULL)
        lastroom = room;

    total += score;
    count++;
}

void
text_task_finish(void)
{
    if (travel > 0)
        printf("\nTotal distance travelled: %d\n", travel);
    if (total > 0)
        printf("\nTotal score: %d\n", total);
}
