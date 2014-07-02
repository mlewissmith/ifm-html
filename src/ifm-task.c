/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Task building functions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <vars.h>

#include "ifm-main.h"
#include "ifm-map.h"
#include "ifm-path.h"
#include "ifm-task.h"
#include "ifm-util.h"
#include "ifm-vars.h"

#define MOVETO(room) do {                               \
    location = room;                                    \
    if (location != NULL)                               \
        location_desc = vh_sgetref(location, "DESC");   \
    else                                                \
        location_desc = "nowhere";                      \
} while (0)

/* Task step attributes */
static char *taskattr[] = {
    "TAG", "STYLE", "CMD", "DROPALL", "HIDDEN", "DO", "DROP", "DROPROOM",
    "DROPUNTIL", "GET", "GIVE", "GOTO", "LOSE", "NEED", NULL
};

/* Task step flags */
enum {
    TS_INVALID, TS_IGNORED, TS_UNSAFE, TS_SAFE
};

/* Task step list */
vlist *tasklist = NULL;

/* Current location */
static vhash *location = NULL;
static char *location_desc = "nowhere";

/* Control variables */
static int all_tasks_safe = 0;
static int keep_unused_items = 0;

/* Internal functions */
static void add_task(vhash *task);
static int do_task(vhash *task, int print, int recurse);
static void drop_item(vhash *item, vhash *room, vlist *until, int print);
static void filter_tasks(int print);
static vhash *first_task(vhash *step);
static void goto_room(vhash *task);
static void invert_items(vhash *obj, char *attr);
static void mark_finishing(char *action, char *otype, char *varname,
                           vhash *table);
static vhash *new_task(int type, vhash *data);
static void order_tasks(vhash *before, vhash *after);
static int task_status(vhash *room, vhash *step);
static int want_item(vhash *item);
static void warn_failure(void);

/* Add a task to the task list (if not already there) */
static void
add_task(vhash *task)
{
    if (!vh_exists(task, "DEPEND")) {
        if (tasklist == NULL)
            tasklist = vl_create();

        vl_ppush(tasklist, task);
        vh_pstore(task, "DEPEND", vl_create());
    }
}

/* Check task list for cyclic dependencies */
void
check_cycles(void)
{
    vlist *tsort, *cycles;
    char *line, *id;
    vhash *step;
    vlist *list;
    V_BUF_DECL;
    viter i, j;
    vgraph *g;
    int count;

    /* Don't bother if no tasks */
    if (tasklist == NULL || vl_length(tasklist) == 0)
        return;

    /* Build task graph */
    g = task_graph();

    /* Do topological sort */
    if ((tsort = vg_tsort(g)) != NULL) {
        vl_destroy(tsort);
        vg_destroy(g);
        return;
    }

    /* Cycles found -- give error and die */
    V_BUF_INIT;
    cycles = vg_tsort_cycles();

    v_iterate(cycles, i) {
        vscalar *elt;
        list = vl_iter_pval(i);
        v_iterate(list, j) {
            elt = vl_iter_val(j);
            id = vl_iter_svalref(j);
            step = vg_node_pget(g, id);
            vs_sstore(elt, vh_sgetref(step, "DESC"));
        }

        line = vl_join(list, " -> ");
        list = vl_filltext(line, 65);

        V_BUF_ADD("   cycle:\n");
        v_iterate(list, j)
            V_BUF_ADD1("      %s\n", vl_iter_svalref(j));
    }

    count = vl_length(cycles);
    err("can't solve game (%d cyclic task dependenc%s)\n%s",
        count, (count == 1 ? "y" : "ies"), V_BUF_VAL);
}

/* Perform a task */
static int
do_task(vhash *task, int print, int recurse)
{
    int scoretask = 1, score, filter = 0;
    vhash *item, *room, *otask, *step;
    vlist *list, *until;
    viter iter;

    if (vh_iget(task, "DONE"))
        return 1;

    /* Do the task */
    switch (vh_iget(task, "TYPE")) {

    case T_GET:
        item = vh_pget(task, "DATA");
        if (vh_exists(item, "TAKEN"))
            scoretask = 0;

        vh_istore(item, "TAKEN", 1);
        if (vh_iget(item, "GIVEN"))
            print = 0;

        if (vh_iget(item, "FINISH"))
            add_attr(task, "NOTE", "Finishes the game");
        else if (!vh_iget(item, "USED"))
            add_attr(task, "NOTE", "Not used for anything yet");

        break;

    case T_DROP:
        item = vh_pget(task, "DATA");
        vh_istore(item, "TAKEN", 0);
        solver_msg(3, "drop item: %s", vh_sgetref(item, "DESC"));
        if (vh_iget(item, "LOST"))
            print = 0;

        break;

    case T_USER:
        if ((list = vh_pget(task, "GIVE")) != NULL) {
            v_iterate(list, iter) {
                item = vl_iter_pval(iter);
                add_attr(task, "NOTE", "Gives %s", vh_sgetref(item, "DESC"));
                if (!vh_iget(item, "USED"))
                    add_attr(task, "NOTE", "Not used for anything yet");
            }
        }

        if ((room = vh_pget(task, "GOTO")) != NULL)
            add_attr(task, "NOTE", "Moves you to %s",
                     vh_sgetref(room, "DESC"));

        if (vh_iget(task, "FINISH"))
            add_attr(task, "NOTE", "Finishes the game");

        filter = 1;
        break;

    case T_GOTO:
        print = 0;
        break;
    }

    if (!scoretask)
        vh_delete(task, "SCORE");

    if (print || recurse) {
        vl_ppush(taskorder, task);
        solver_msg(2, "do task: %s", vh_sgetref(task, "DESC"));
    }

    /* Flag it as done */
    vh_istore(task, "DONE", 1);

    /* Get any given items */
    if ((list = vh_pget(task, "GIVE")) != NULL) {
        v_iterate(list, iter) {
            item = vl_iter_pval(iter);

            if (!vh_exists(item, "TAKEN")) {
                score = vh_iget(task, "SCORE") + vh_iget(item, "SCORE");
                vh_istore(task, "SCORE", score);
            }

            solver_msg(3, "give item: %s", vh_sgetref(item, "DESC"));
            vh_istore(item, "TAKEN", 1);
        }
    }

    /* Lose any lost items */
    if ((list = vh_pget(task, "LOSE")) != NULL) {
        v_iterate(list, iter) {
            item = vl_iter_pval(iter);
            solver_msg(3, "lose item: %s", vh_sgetref(item, "DESC"));
            vh_istore(item, "TAKEN", 0);
        }
    }

    /* Drop any dropped items */
    if ((list = vh_pget(task, "DROP")) != NULL) {
        if ((room = vh_pget(task, "DROPROOM")) == NULL)
            room = location;

        until = vh_pget(task, "DROPUNTIL");

        v_iterate(list, iter) {
            item = vl_iter_pval(iter);
            drop_item(item, room, until, 0);
        }
    }

    /* Do any other tasks */
    if ((list = vh_pget(task, "DO")) != NULL) {
        v_iterate(list, iter) {
            otask = vl_iter_pval(iter);
            step = vh_pget(otask, "STEP");
            if (vh_iget(step, "DONE"))
                continue;

            if (vh_iget(step, "MODPATH")) {
                vh_delete(step, "MODPATH");
                vh_istore(task, "MODPATH", 1);
            }

            solver_msg(3, "also do: %s", vh_sgetref(step, "DESC"));

            add_attr(step, "CMD", NULL);
            vh_pstore(step, "ROOM", location);

            if (!do_task(step, 0, 1))
                vh_istore(task, "FINISH", 1);
        }
    }

    /* Teleport to new location if required */
    if ((room = vh_pget(task, "GOTO")) != NULL) {
        solver_msg(2, "goto room: %s", vh_sgetref(room, "DESC"));
        MOVETO(room);
    }

    /* Flag path modification if required */
    if (vh_iget(task, "MODPATH"))
        modify_path(print);

    /* Flag redundant tasks as done */
    if (filter)
        filter_tasks(print);

    /* Return whether to try more tasks */
    if (vh_iget(task, "FINISH"))
        return 0;

    if (location != NULL && vh_iget(location, "FINISH"))
        return 0;

    return 1;
}

/* Leave an item in a room and maybe get it later */
static void
drop_item(vhash *item, vhash *room, vlist *until, int print)
{
    vhash *tstep, *step, *task;
    vlist *list;
    viter iter;

    /* Do nothing if not carrying it */
    if (!vh_iget(item, "TAKEN"))
        return;

    /* Drop the item */
    vh_pstore(item, "ROOM", room);
    step = new_task(T_DROP, item);
    do_task(step, print, 0);

    /* Create new task step to get it back */
    step = new_task(T_GET, item);

    /* Add new task dependencies (if any) */
    if (until != NULL) {
        v_iterate(until, iter) {
            task = vl_iter_pval(iter);
            tstep = vh_pget(task, "STEP");
            if (!vh_iget(tstep, "DONE"))
                order_tasks(tstep, step);
        }
    }

    if ((list = vh_pget(item, "TASKS")) != NULL) {
        v_iterate(list, iter) {
            tstep = vl_iter_pval(iter);
            if (!vh_iget(tstep, "DONE"))
                order_tasks(step, tstep);
        }
    }

    /* If the item is still wanted, flag an optional retrieval */
    if (want_item(item)) {
        add_task(step);
        vh_istore(step, "OPTIONAL", 1);
    }
}

/* Filter redundant tasks from the task list */
static void
filter_tasks(int print)
{
    int canfilter, filter, filtered, numfiltered = 0, alldone;
    vhash *task, *otask, *item, *step;
    char *reason;
    vlist *list;
    viter i, j;

    /* Loop until no more filtering possible */
    do {
        filtered = 0;

        v_iterate(tasklist, i) {
            task = vl_iter_pval(i);

            /* By default, can't filter */
            canfilter = 0;

            /* But we want to */
            filter = 1;

            /* Check simple non-filtering cases */
            if (vh_iget(task, "DONE") || vh_iget(task, "FINISH") ||
                vh_iget(task, "SCORE") || vh_iget(task, "MODPATH") ||
                vh_exists(task, "NEXT"))
                filter = 0;

            /* Can't filter if task allows other tasks */
            if (filter && (list = vh_pget(task, "ALLOW")) != NULL) {
                alldone = 1;

                v_iterate(list, j) {
                    otask = vl_iter_pval(j);
                    if (!vh_iget(otask, "DONE"))
                        alldone = 0;
                }

                if (!alldone)
                    filter = 0;
            }

            /* Can't filter if task gets new items */
            if (filter && (list = vh_pget(task, "GET")) != NULL) {
                alldone = 1;
                canfilter = 1;
                reason = "items already carried";

                v_iterate(list, j) {
                    item = vl_iter_pval(j);
                    if (!vh_exists(item, "TAKEN"))
                        alldone = 0;
                }

                if (!alldone)
                    filter = 0;
            }

            /* Can't filter if task gives new items */
            if (filter && (list = vh_pget(task, "GIVE")) != NULL) {
                alldone = 1;
                canfilter = 1;
                reason = "items already carried";

                v_iterate(list, j) {
                    item = vl_iter_pval(j);
                    if (!vh_exists(item, "TAKEN"))
                        alldone = 0;
                }

                if (!alldone)
                    filter = 0;
            }

            /* Can filter do-task tasks if they're all done */
            if (filter && (list = vh_pget(task, "DO")) != NULL) {
                alldone = 1;
                reason = "tasks already done";

                v_iterate(list, j) {
                    otask = vl_iter_pval(j);
                    step = vh_pget(otask, "STEP");
                    if (!vh_iget(step, "DONE"))
                        alldone = 0;
                }

                if (alldone)
                    canfilter = 1;
            }

            /* Can filter get-item tasks for carried items */
            if (!vh_iget(task, "DONE") && vh_iget(task, "TYPE") == T_GET) {
                canfilter = 1;
                reason = "item already carried";
                item = vh_pget(task, "DATA");
                filter = vh_iget(item, "TAKEN");
            }

            /* Filter if required */
            if (canfilter && filter) {
                filtered++;
                numfiltered++;
                vh_istore(task, "DONE", 1);
                solver_msg(3, "redundant task: %s (%s)",
                           vh_sgetref(task, "DESC"), reason);
            }
        }
    } while (filtered);

    if (numfiltered)
        modify_path(print);
}

/* Return first task in a task's follow-chain */
static vhash *
first_task(vhash *step)
{
    vhash *first, *prev;

    if ((first = vh_pget(step, "PREV")) != NULL) {
        while ((prev = vh_pget(first, "PREV")) != NULL)
            first = prev;
    } else {
        first = step;
    }

    return first;
}

/* Move to a room to do something */
static void
goto_room(vhash *task)
{
    vhash *item, *reach, *mtask, *last, *room;
    vlist *path, *list;
    viter i, j;

    /* See if movement is required */
    if ((room = vh_pget(task, "ROOM")) == NULL)
        return;
    if (room == location)
        return;

    /* Add movement tasks */
    path = get_path(task, room);
    last = location;

    v_iterate(path, i) {
        reach = vl_iter_pval(i);
        room = vh_pget(reach, "TO");

        /* Leave items behind if required */
        if ((list = vh_pget(reach, "LEAVE")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                drop_item(item, last, NULL, 1);
            }
        }

        if ((list = vh_pget(room, "LEAVE")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                drop_item(item, last, NULL, 1);
            }
        }

        /* Move to the next room */
        mtask = new_task(T_MOVE, room);
        list = vh_pget(reach, "CMD");
        vh_pstore(mtask, "CMD", list);
        vl_ppush(taskorder, mtask);

        if (vh_exists(room, "VISITED")) {
            vh_delete(mtask, "SCORE");
            vh_delete(mtask, "NOTE");
        }

        vh_istore(room, "VISITED", 1);

        if (vh_iget(room, "FINISH"))
            add_attr(mtask, "NOTE", "Finishes the game");

        solver_msg(2, "move to: %s", vh_sgetref(room, "DESC"));
        last = room;
    }

    MOVETO(room);
}

/* Invert an item list if required */
static void
invert_items(vhash *obj, char *attr)
{
    vlist *list, *newlist = NULL;
    static int num = 0;
    vhash *item;
    viter iter;

    list = vh_pget(obj, attr);
    num++;

    if (list != NULL) {
        v_iterate(list, iter) {
            item = vl_iter_pval(iter);
            vh_istore(item, "INVERT", num);
        }
    }

    v_iterate(items, iter) {
        item = vl_iter_pval(iter);
        if (vh_iget(item, "INVERT") == num)
            continue;

        if (newlist == NULL)
            newlist = vl_create();

        vl_ppush(newlist, item);
    }

    vh_pstore(obj, attr, newlist);
}

/* Mark an object as finishing the game */
static void
mark_finishing(char *action, char *otype, char *varname, vhash *table)
{
    char *value, *tag, *name;
    vlist *tags;
    vhash *obj;
    viter i;

    value = var_string(varname);
    tags = vl_split(value, ",");

    v_iterate(tags, i) {
        tag = vl_iter_sval(i);
        if ((obj = vh_pget(table, tag)) != NULL) {
            name = vh_sgetref(obj, "DESC");
            solver_msg(2, "%s %s '%s'", action, otype, name);
            vh_istore(obj, "FINISH", 1);
        } else {
            err("%s tag '%s' not defined", otype, tag);
        }
    }

    vl_destroy(tags);
}

/* Create and return a new task step */
static vhash *
new_task(int type, vhash *data)
{
    char *desc = vh_sgetref(data, "DESC");
    int i, score = vh_iget(data, "SCORE");
    static int taskid = 0;
    vhash *room, *step;
    vscalar *val;
    V_BUF_DECL;

    step = vh_create();
    vh_istore(step, "TYPE", type);
    vh_pstore(step, "DATA", data);

    switch (type) {
    case T_MOVE:
        V_BUF_SET1("Move to %s", desc);
        room = data;
        break;
    case T_GET:
        V_BUF_SET1("Get %s", desc);
        room = vh_pget(data, "ROOM");
        break;
    case T_DROP:
        V_BUF_SET1("Drop %s", desc);
        room = NULL;
        score = 0;
        break;
    case T_GOTO:
        V_BUF_SET1("Go to %s", desc);
        room = data;
        break;
    case T_USER:
        V_BUF_SET(desc);
        room = vh_pget(data, "ROOM");

        for (i = 0; taskattr[i] != NULL; i++)
            if ((val = vh_get(data, taskattr[i])) != NULL)
                vh_store(step, taskattr[i], vs_copy(val));

        break;
    default:
        fatal("internal: unknown task type");
    }

    if (vh_exists(data, "SAFE"))
        vh_istore(step, "SAFE", vh_iget(data, "SAFE"));

    if (vh_exists(data, "FINISH"))
        vh_istore(step, "FINISH", vh_iget(data, "FINISH"));

    if (vh_exists(data, "NOTE") && type != T_DROP)
        vh_pstore(step, "NOTE", vh_pget(data, "NOTE"));

    vh_istore(step, "IGNORE", vh_iget(data, "IGNORE"));

    vh_sstore(step, "DESC", V_BUF_VAL);
    vh_pstore(step, "ROOM", room);
    vh_istore(step, "SCORE", score);
    vh_istore(step, "ID", taskid++);

    return step;
}

/* Add an ordered task pair to the task list */
static void
order_tasks(vhash *before, vhash *after)
{
    vlist *allow, *depend;
    int count = 0;
    vhash *step;

    /* Add the tasks */
    add_task(before);
    add_task(after);

    if (before != after) {
        /* The 'before' task allows the 'after' one to be done */
        add_list(before, "ALLOW", after);

        /* The 'after' task (and previous ones in its follow-chain)
         * depends on the 'before' one */
        for (step = after; step != NULL; step = vh_pget(step, "PREV")) {
            if (step != before) {
                add_list(step, "DEPEND", before);
                solver_msg(2, "task order: do '%s' before '%s'",
                           vh_sgetref(before, "DESC"),
                           vh_sgetref(step, "DESC"));
            }

            /* Infinite-loop paranoia check */
            if (step == after && count++ > 0)
                break;
        }
    }
}

/* Return a task required by a given task, if any */
vhash *
require_task(vhash *step)
{
    vhash *before;
    vlist *depend;
    viter iter;

    if ((depend = vh_pget(step, "DEPEND")) == NULL)
        return NULL;

    v_iterate(depend, iter) {
        before = vl_iter_pval(iter);
        if (!vh_iget(before, "DONE"))
            return before;
    }

    return NULL;
}

/* Build the initial task list */
void
setup_tasks(void)
{
    vhash *task, *otask, *get, *after, *item, *tstep, *room, *reach;
    vhash *step, *istep, *oitem, *first;
    vlist *list, *itasks, *rlist;
    int flagged = 1;
    viter i, j, k;
    char *msg;

    solver_msg(0, "\nSetting up tasks...");

    /* Flag any extra rooms/items/tasks as finishing things */
    solver_msg(1, "Marking extra events that finish the game");
    mark_finishing("entering", "room", "finish_room", roomtags);
    mark_finishing("obtaining", "item", "finish_item", itemtags);
    mark_finishing("doing", "task", "finish_task", tasktags);

    /* Create 'goto room' steps */
    v_iterate(rooms, i) {
        room = vl_iter_pval(i);
        step = new_task(T_GOTO, room);
        vh_pstore(room, "STEP", step);
        if (vh_iget(room, "SCORE") > 0 || vh_iget(room, "FINISH"))
            add_task(step);
    }

    /* Create 'get item' steps */
    v_iterate(items, i) {
        item = vl_iter_pval(i);
        step = new_task(T_GET, item);
        vh_pstore(item, "STEP", step);
        if (vh_iget(item, "SCORE") > 0 || vh_iget(item, "FINISH"))
            add_task(step);
    }

    /* Create user task steps */
    v_iterate(tasks, i) {
        task = vl_iter_pval(i);
        step = new_task(T_USER, task);
        vh_pstore(task, "STEP", step);
        add_task(step);
    }

    /* Set up task 'follow' links */
    solver_msg(1, "Adding dependencies for task 'follow' entries");

    v_iterate(tasks, i) {
        task = vl_iter_pval(i);
        tstep = vh_pget(task, "STEP");

        if ((otask = vh_pget(task, "FOLLOW")) != NULL) {
            step = vh_pget(otask, "STEP");
            if (vh_exists(step, "NEXT") && vh_pget(step, "NEXT") != tstep)
                err("more than one task needs to follow '%s' immediately",
                    vh_sgetref(otask, "DESC"));

            order_tasks(step, tstep);
            vh_pstore(step, "NEXT", tstep);
            vh_pstore(tstep, "PREV", step);
        }
    }

    /* Process rooms and map connections */
    v_iterate(rooms, i) {
        room = vl_iter_pval(i);
        rlist = reachable_rooms(room);

        /* Flag 'before' tasks for this room as unsafe */
        if ((list = vh_pget(room, "BEFORE")) != NULL) {
            v_iterate(list, j) {
                task = vl_iter_pval(j);
                step = vh_pget(task, "STEP");
                vh_istore(step, "MODPATH", 1);
                vh_sstore(step, "UNSAFE", "closes off room");
            }
        }

        /* Mention 'after' tasks for this room */
        if ((list = vh_pget(room, "AFTER")) != NULL) {
            v_iterate(list, j) {
                task = vl_iter_pval(j);
                step = vh_pget(task, "STEP");
                vh_istore(step, "MODPATH", 1);
                add_task(step);
            }
        }

        /* Invert room 'leave' list if leaving all */
        if (vh_iget(room, "LEAVEALL"))
            invert_items(room, "LEAVE");

        /* Flag items which might have to be left before entering room */
        if ((list = vh_pget(room, "LEAVE")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                vh_istore(item, "LEAVE", 1);
                step = vh_pget(item, "STEP");
                vh_istore(step, "MODPATH", 1);
            }
        }

        /* Must get required items before visiting this room */
        if ((list = vh_pget(room, "NEED")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                vh_istore(item, "USED", 1);
                vh_istore(item, "NEEDED", 1);

                room = vh_pget(item, "ROOM");
                if (room != NULL) {
                    step = vh_pget(item, "STEP");
                    vh_istore(step, "MODPATH", 1);
                    add_task(step);
                    add_list(item, "NROOMS", room);
                }
            }
        }

        v_iterate(rlist, j) {
            reach = vl_iter_pval(j);

            /* Invert reach 'leave' list if leaving all */
            if (vh_iget(reach, "LEAVEALL"))
                invert_items(reach, "LEAVE");

            /* Flag items which might have to be left before using link */
            if ((list = vh_pget(reach, "LEAVE")) != NULL) {
                v_iterate(list, k) {
                    item = vl_iter_pval(k);
                    vh_istore(item, "LEAVE", 1);
                    step = vh_pget(item, "STEP");
                    vh_istore(step, "MODPATH", 1);
                }
            }

            /* Flag 'before' tasks for this connection as unsafe */
            if ((list = vh_pget(reach, "BEFORE")) != NULL) {
                v_iterate(list, k) {
                    task = vl_iter_pval(k);
                    step = vh_pget(task, "STEP");
                    vh_istore(step, "MODPATH", 1);
                    vh_sstore(step, "UNSAFE", "closes off link");
                }
            }

            /* Mention 'after' tasks for this connection */
            if ((list = vh_pget(reach, "AFTER")) != NULL) {
                v_iterate(list, k) {
                    task = vl_iter_pval(k);
                    step = vh_pget(task, "STEP");
                    vh_istore(step, "MODPATH", 1);
                    add_task(step);
                }
            }

            /* Flag items which are needed for paths */
            if ((list = vh_pget(reach, "NEED")) != NULL) {
                v_iterate(list, k) {
                    item = vl_iter_pval(k);
                    vh_istore(item, "USED", 1);
                    vh_istore(item, "NEEDED", 1);
                    step = vh_pget(item, "STEP");
                    vh_istore(step, "MODPATH", 1);
                    add_task(step);
                    add_list(item, "NLINKS", reach);
                }
            }
        }
    }

    /* Process items */
    solver_msg(1, "Adding dependencies for item 'before/after' lists");

    v_iterate(items, i) {
        item = vl_iter_pval(i);
        istep = vh_pget(item, "STEP");

        /* Mark item as taken if it starts off that way */
        if ((room = vh_pget(item, "ROOM")) == NULL)
            vh_istore(item, "TAKEN", 1);

        /* Must get required items before getting this item */
        if ((list = vh_pget(item, "NEED")) != NULL) {
            v_iterate(list, j) {
                oitem = vl_iter_pval(j);
                vh_istore(oitem, "USED", 1);

                room = vh_pget(oitem, "ROOM");
                if (room != NULL) {
                    get = vh_pget(oitem, "STEP");
                    order_tasks(get, istep);
                    add_list(item, "RTASKS", get);
                }

                add_list(oitem, "TASKS", istep);
            }
        }

        /* Deal with 'after' tasks */
        if ((list = vh_pget(item, "AFTER")) != NULL) {
            v_iterate(list, j) {
                task = vl_iter_pval(j);
                step = vh_pget(task, "STEP");
                add_list(item, "RTASKS", step);
                order_tasks(step, istep);
            }
        }

        /* Deal with 'before' tasks */
        if ((list = vh_pget(item, "BEFORE")) != NULL) {
            v_iterate(list, j) {
                task = vl_iter_pval(j);
                step = vh_pget(task, "STEP");
                add_list(item, "RTASKS", step);
                order_tasks(istep, step);
            }
        }
    }

    solver_msg(1, "Adding dependencies for task 'need/get/give/after' lists");

    /* Process task 'need/get/give/after/drop' stuff */
    v_iterate(tasks, i) {
        task = vl_iter_pval(i);
        tstep = vh_pget(task, "STEP");
        first = first_task(tstep);

        /* Invert task 'drop' list if dropping all */
        if (vh_iget(task, "DROPALL"))
            invert_items(tstep, "DROP");

        /* Must get required items before doing first task in follow-chain */
        if ((list = vh_pget(task, "NEED")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                vh_istore(item, "USED", 1);

                if ((room = vh_pget(item, "ROOM")) != NULL) {
                    get = vh_pget(item, "STEP");
                    order_tasks(get, tstep);
                }

                add_list(item, "TASKS", first);
            }
        }

        /* Must do this task before giving or getting supplied items */
        if ((list = vh_pget(task, "GET")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                if ((room = vh_pget(item, "ROOM")) != NULL) {
                    get = vh_pget(item, "STEP");
                    add_list(item, "RTASKS", tstep);
                    order_tasks(tstep, get);
                }
            }
        }

        if ((list = vh_pget(task, "GIVE")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                if ((room = vh_pget(item, "ROOM")) != NULL) {
                    get = vh_pget(item, "STEP");
                    add_list(item, "RTASKS", tstep);
                    order_tasks(tstep, get);
                }
            }
        }

        /* Must do this task after all preceding tasks */
        if ((list = vh_pget(task, "AFTER")) != NULL) {
            v_iterate(list, j) {
                otask = vl_iter_pval(j);
                after = vh_pget(otask, "STEP");
                order_tasks(after, tstep);
            }
        }
    }

    /* Process task 'lose' stuff */
    solver_msg(1, "Adding dependencies for task 'lose' lists");

    v_iterate(tasks, i) {
        task = vl_iter_pval(i);
        tstep = vh_pget(task, "STEP");

        if ((list = vh_pget(tstep, "LOSE")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);

                if ((itasks = vh_pget(item, "TASKS")) != NULL) {
                    v_iterate(itasks, k) {
                        step = vl_iter_pval(k);
                        order_tasks(step, tstep);
                    }
                }

                /* If item is needed for paths, mark task unsafe */
                if (vh_exists(item, "NEEDED"))
                    vh_sstore(tstep, "UNSAFE", "loses item needed for paths");
            }
        }
    }

    /* Propagate 'unsafe' flags for 'follow' tasks */
    v_iterate(tasks, i) {
        task = vl_iter_pval(i);
        tstep = vh_pget(task, "STEP");

        if (vh_exists(tstep, "PREV") &&
            vh_exists(tstep, "UNSAFE") &&
            !vh_iget(tstep, "SAFE")) {
            step = vh_pget(tstep, "PREV");
            msg = vh_sgetref(tstep, "UNSAFE");
            while (step != NULL) {
                vh_sstore(step, "UNSAFE", msg);
                step = vh_pget(step, "PREV");
            }
        }
    }

    /* Propagate 'unsafe' flags for 'do' tasks */
    while (flagged) {
        flagged = 0;

        v_iterate(tasks, i) {
            task = vl_iter_pval(i);
            tstep = vh_pget(task, "STEP");
            if (vh_defined(tstep, "UNSAFE"))
                continue;

            if ((list = vh_pget(tstep, "DO")) == NULL)
                continue;

            v_iterate(list, j) {
                otask = vl_iter_pval(j);
                step = vh_pget(otask, "STEP");
                if (vh_defined(step, "UNSAFE")) {
                    vh_sstore(tstep, "UNSAFE", "does unsafe task");
                    flagged = 1;
                }
            }
        }
    }
}

/* Solve the game by ordering the task list */
void
solve_game(void)
{
    int count, tasksleft, status, ignore = 0;
    vhash *step, *trystep, *item, *next;
    viter iter;

    /* Don't bother if no tasks */
    if (tasklist == NULL || vl_length(tasklist) == 0)
        return;

    /* Set control variables */
    all_tasks_safe = var_int("all_tasks_safe");
    keep_unused_items = var_int("keep_unused_items");

    /* Build initial inventory */
    v_iterate(items, iter) {
        item = vl_iter_pval(iter);
        if (vh_pget(item, "ROOM") == NULL)
            vh_istore(item, "TAKEN", 1);
    }

    /* Process task list */
    MOVETO(startroom);
    next = NULL;

    solver_msg(0, "\nSolving game...");

    do {
        solver_msg(1, "Location: %s", location_desc);

        /* Initialise path searches from this room */
        if (location != NULL)
            init_path(location);

        /* Check for dropping unneeded items */
        if (next == NULL &&
            (location == NULL || !vh_iget(location, "NODROP"))) {
            while (1) {
                count = 0;

                v_iterate(items, iter) {
                    item = vl_iter_pval(iter);

                    /* Skip if not carried */
                    if (!vh_iget(item, "TAKEN"))
                        continue;

                    /* Skip if not used yet */
                    if (keep_unused_items && !vh_iget(item, "USED"))
                        continue;

                    /* Skip if wanted */
                    if (want_item(item))
                        continue;

                    /* Nah, dump it */
                    step = new_task(T_DROP, item);
                    do_task(step, 1, 0);
                    count++;
                }

                if (count == 0)
                    break;
            }
        }

        /* Search for next task */
        step = NULL;
        tasksleft = 0;

        v_iterate(tasklist, iter) {
            trystep = vl_iter_pval(iter);

            /* If task is done, skip it */
            if (vh_iget(trystep, "DONE"))
                continue;

            /* If not optional, flag at least one task still to do */
            if (!vh_iget(trystep, "OPTIONAL"))
                tasksleft = 1;

            /* If there's a forced next task but this isn't it, skip it */
            if (next != NULL && trystep != next)
                continue;

            /* Check task is possible */
            if ((status = task_status(location, trystep)) == TS_INVALID)
                continue;

            /* Check task isn't ignored */
            if (status == TS_IGNORED) {
                ignore++;
                continue;
            }

            if (status == TS_SAFE || all_tasks_safe) {
                /* A safe task -- choose it */
                step = trystep;
                break;
            } else if (step == NULL) {
                /* The closest unsafe task */
                step = trystep;
            }
        }

        if (step != NULL) {
            /* Do the task */
            goto_room(step);
            tasksleft = do_task(step, 1, 0);
            next = vh_pget(step, "NEXT");
        } else if (tasksleft) {
            /* Hmm... we seem to be stuck */
            warn_failure();
            if (ignore)
                solver_msg(2, "%d ignored tasks", ignore);
            break;
        } else {
            solver_msg(2, "no more tasks");
        }
    } while (tasksleft);

    solver_msg(0, "");
}

/* Print solver message */
void
solver_msg(int level, char *fmt, ...)
{
    V_BUF_DECL;
    char *msg;

    if (TASK_VERBOSE) {
        indent(level);
        V_BUF_FMT(fmt, msg);
        printf("%s\n", msg);
    }
}

/* Build task dependency graph */
vgraph *
task_graph(void)
{
    char *before, *after;
    int count = 0;
    vlist *list;
    vhash *step;
    V_BUF_DECL;
    viter i, j;
    vgraph *g;

    g = vg_create();

    if (tasklist != NULL) {
        v_iterate(tasklist, i) {
            step = vl_iter_pval(i);
            V_BUF_SET1("T%d", ++count);
            vh_sstore(step, "NODE", V_BUF_VAL);
            vg_node_pstore(g, V_BUF_VAL, step);
        }

        v_iterate(tasklist, i) {
            step = vl_iter_pval(i);
            if ((list = vh_pget(step, "DEPEND")) == NULL)
                continue;

            after = vh_sgetref(step, "NODE");
            v_iterate(list, j) {
                step = vl_iter_pval(j);
                before = vh_sgetref(step, "NODE");
                vg_link_oneway(g, before, after);
            }
        }
    }

    return g;
}

/* Return current status of a task */
static int
task_status(vhash *room, vhash *step)
{
    vhash *taskroom, *gotoroom, *droproom;
    char *safemsg = NULL;
    int len = 0;

    /* All dependent tasks must be done */
    if (require_task(step) != NULL)
        return TS_INVALID;

    /* Task must not be ignored */
    if (vh_iget(step, "IGNORED"))
        return TS_INVALID;

    if (vh_iget(step, "IGNORE")) {
        if (!vh_iget(step, "IGNORED")) {
            solver_msg(2, "consider: %s", vh_sgetref(step, "DESC"));
            solver_msg(3, "not possible: explicitly ignored");
            vh_istore(step, "IGNORED", 1);
        }

        return TS_IGNORED;
    }

    solver_msg(2, "consider: %s", vh_sgetref(step, "DESC"));

    /* If task is done elsewhere, make sure you can get there */
    taskroom = vh_pget(step, "ROOM");
    gotoroom = vh_pget(step, "GOTO");

    if (taskroom != NULL && room != NULL && taskroom != room)
        if ((len = find_path(step, room, taskroom)) == NOPATH)
            return TS_INVALID;

    /* See whether task is safe */
    if (vh_exists(step, "SAFE")) {
        /* User says it's safe, and they know best */
    } else if (vh_exists(step, "UNSAFE")) {
        /* It's been flagged unsafe already */
        safemsg = vh_sgetref(step, "UNSAFE");
    } else {
        /* If no return path, mark it as unsafe */
        if (gotoroom == NULL)
            gotoroom = taskroom;

        if (gotoroom != NULL && find_path(NULL, gotoroom, room) == NOPATH)
            safemsg = "no return path";

        /* If task drops items, check you can get them back */
        if (vh_pget(step, "DROP") != NULL) {
            droproom = vh_pget(step, "DROPROOM");

            if (droproom == NULL)
                droproom = taskroom;

            if (droproom == NULL)
                droproom = location;

            if (find_path(NULL, gotoroom, droproom) == NOPATH)
                safemsg = "no path to dropped items";
        }
    }

    if (TASK_VERBOSE) {
        indent(2);
        printf("possible: %s", vh_sgetref(step, "DESC"));

        if (len > 0)
            printf(" (dist %d)", len);

        if (safemsg != NULL)
            printf(" (unsafe: %s)", safemsg);

        printf("\n");
    }

    return (safemsg == NULL ? TS_SAFE : TS_UNSAFE);
}

/* Return whether an item is wanted */
static int
want_item(vhash *item)
{
    vhash *kitem, *task, *step;
    vlist *tasks, *kitems;
    viter iter;

    /* Yes if needed for movement */
    if (vh_iget(item, "NEEDED"))
        return 1;

    /* Yes if it's an unconditionally kept item */
    if (vh_iget(item, "KEEP"))
        return 1;

    /* Yes if at least one 'keep with' item is held */
    if ((kitems = vh_pget(item, "KEEP_WITH")) != NULL) {
        v_iterate(kitems, iter) {
            kitem = vl_iter_pval(iter);
            if (!vh_exists(kitem, "TAKEN") || vh_iget(kitem, "TAKEN"))
                return 1;
        }
    }

    /* Yes if at least one 'keep until' task isn't done yet */
    if ((tasks = vh_pget(item, "KEEP_UNTIL")) != NULL) {
        v_iterate(tasks, iter) {
            task = vl_iter_pval(iter);
            step = vh_pget(task, "STEP");
            if (!vh_iget(step, "DONE"))
                return 1;
        }
    }

    /* Yes if it's needed for at least one task */
    if ((tasks = vh_pget(item, "TASKS")) != NULL) {
        v_iterate(tasks, iter) {
            task = vl_iter_pval(iter);
            if (!vh_iget(task, "DONE"))
                return 1;
        }
    }

    /* Otherwise, no */
    return 0;
}

/* Warn about failure to solve the game */
static void
warn_failure(void)
{
    char *tdesc, *rdesc, *reason, *entries;
    vhash *step, *room, *reasons;
    vlist *list, *keys;
    int count = 0;
    V_BUF_DECL;
    viter iter;

    /* Record which tasks can't be done */
    reasons = vh_create();
    v_iterate(tasklist, iter) {
        step = vl_iter_pval(iter);
        if (vh_iget(step, "DONE"))
            continue;

        count++;
        tdesc = vh_sgetref(step, "DESC");

        /* Build failure reason */
        if (vh_iget(step, "IGNORED")) {
            V_BUF_SET("ignored");
            rdesc = NULL;
        } else if (require_task(step) != NULL) {
            V_BUF_SET("requires previous task to be done first");
            rdesc = NULL;
        } else {
            V_BUF_SET("no path to task room");
            room = vh_pget(step, "ROOM");
            rdesc = vh_sgetref(room, "DESC");
        }

        /* Add it to list */
        list = vh_add_list(reasons, V_BUF_VAL);

        if (rdesc == NULL)
            V_BUF_SET(tdesc);
        else
            V_BUF_SET2("%s (%s)", tdesc, rdesc);

        vl_spush(list, V_BUF_VAL);
    }

    /* Build warning message */
    V_BUF_SET1("   final location:\n      %s\n",
               vh_sgetref(location, "DESC"));

    keys = vh_keys(reasons);

    v_iterate(keys, iter) {
        reason = vl_iter_svalref(iter);

        list = vh_pget(reasons, reason);
        entries = vl_join(list, ", ");

        list = vl_filltext(entries, 65);
        V_BUF_ADD1("   %s:\n", reason);

        v_iterate(list, iter)
            V_BUF_ADD1("      %s\n", vl_iter_svalref(iter));
    }

    vl_destroy(keys);

    /* Print it */
    warn("can't solve game (%d task%s not done)\n%s",
         count, (count == 1 ? "" : "s"), V_BUF_VAL);
}
