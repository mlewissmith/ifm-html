/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Path building functions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vars.h>

#include "ifm-main.h"
#include "ifm-map.h"
#include "ifm-path.h"
#include "ifm-task.h"
#include "ifm-util.h"

#define NODE(room)           vh_sgetref(room, "NODE")

#define PATH_NODES(r1, r2)   vg_path_nodes(graph, NODE(r1), NODE(r2))
#define PATH_INFO(r1, r2)    vg_path_info(graph, NODE(r1), NODE(r2))
#define PATH_LENGTH(r1, r2)  ((int) vg_path_length(graph, NODE(r1), NODE(r2)))

#define BIG 1000

/* Graph structure */
static vgraph *graph = NULL;

/* Modified-path flag */
static int path_modify = 0;

/* Path cache room */
static vhash *path_room = NULL;

/* Path task */
static vhash *path_task = NULL;

#ifdef SHOW_VISIT
/* Find-path start room */
static vhash *start_room = NULL;
#endif

/* Internal functions */
static void link_rooms(vhash *from, vhash *to, vhash *reach);
static double link_size(char *fnode, char *tnode, vscalar *s);
static int sort_tasks(vscalar **v1, vscalar **v2);
static int use_link(char *fnode, char *tnode, vscalar *s);
static int use_node(char *node, vscalar *s, double dist);

/* Connect rooms as a directed graph */
void
connect_rooms(void)
{
    int oneway, len, goflag, dir, num = 0, uselen = 0;
    vhash *room, *link, *join, *reach, *from, *to;
    vlist *cmdfrom, *cmdto, *list;
    char *cmd, id[10];
    viter iter;

    solver_msg(0, "Connecting rooms...");

    /* Initialise */
    graph = vg_create();

    /* Create room nodes */
    v_iterate(rooms, iter) {
        room = vl_iter_pval(iter);
        sprintf(id, "R%d", ++num);
        vh_sstore(room, "NODE", id);
        vg_node_pstore(graph, id, room);
    }

    /* Build link connections */
    v_iterate(links, iter) {
        link = vl_iter_pval(iter);

        if (vh_iget(link, "NOLINK"))
            continue;

        if (vh_iget(link, "NOPATH"))
            continue;

        oneway = vh_iget(link, "ONEWAY");
        len = vh_iget(link, "LEN");
        if (len)
            uselen++;

        from = vh_pget(link, "FROM");
        to = vh_pget(link, "TO");

        /* From -> to */
        reach = vh_create();

        cmd = NULL;
        goflag = vh_iget(link, "GO");
        dir = vh_iget(link, "TO_DIR");
        if ((cmdto = vh_pget(link, "TO_CMD")) != NULL) {
            vh_pstore(reach, "CMD", cmdto);
        } else {
            if (goflag)
                cmd = dirinfo[goflag].sname;
            if (cmd == NULL)
                cmd = dirinfo[dir].sname;
            add_attr(reach, "CMD", cmd);
        }

        vh_pstore(reach, "FROM", from);
        vh_pstore(reach, "TO", to);
        vh_pstore(reach, "NEED", vh_pget(link, "NEED"));
        vh_pstore(reach, "BEFORE", vh_pget(link, "BEFORE"));
        vh_pstore(reach, "AFTER", vh_pget(link, "AFTER"));
        vh_istore(reach, "LEN", V_MAX(len, 1));
        vh_pstore(reach, "LEAVE", vh_pget(link, "LEAVE"));
        vh_istore(reach, "LEAVEALL", vh_iget(link, "LEAVEALL"));

        link_rooms(from, to, reach);

        if (TASK_VERBOSE) {
            indent(1);
            list = vh_pget(reach, "CMD");
            printf("link '%s' to '%s' (%s)",
                   vh_sgetref(from, "DESC"),
                   vh_sgetref(to, "DESC"),
                   vl_join(list, ". "));
            if (len > 1)
                printf(" (dist %d)", len);
            printf("\n");
        }

        /* To -> from if not one-way */
        if (!oneway) {
            reach = vh_create();

            cmd = NULL;
            goflag = dirinfo[goflag].odir;
            dir = vh_iget(link, "FROM_DIR");
            if ((cmdfrom = vh_pget(link, "FROM_CMD")) != NULL) {
                vh_pstore(reach, "CMD", cmdfrom);
            } else if (cmdto != NULL) {
                vh_pstore(reach, "CMD", cmdto);
            } else {
                if (goflag)
                    cmd = dirinfo[goflag].sname;
                if (cmd == NULL)
                    cmd = dirinfo[dir].sname;
                add_attr(reach, "CMD", cmd);
            }

            vh_pstore(reach, "FROM", to);
            vh_pstore(reach, "TO", from);
            vh_pstore(reach, "NEED", vh_pget(link, "NEED"));
            vh_pstore(reach, "BEFORE", vh_pget(link, "BEFORE"));
            vh_pstore(reach, "AFTER", vh_pget(link, "AFTER"));
            vh_pstore(reach, "LEAVE", vh_pget(link, "LEAVE"));
            vh_istore(reach, "LEAVEALL", vh_iget(link, "LEAVEALL"));
            vh_istore(reach, "LEN", V_MAX(len, 1));

            link_rooms(to, from, reach);

            if (TASK_VERBOSE) {
                indent(1);
                list = vh_pget(reach, "CMD");
                printf("link '%s' to '%s' (%s)",
                       vh_sgetref(to, "DESC"),
                       vh_sgetref(from, "DESC"),
                       vl_join(list, ". "));
                if (len > 1)
                    printf(" (dist %d)", len);
                printf("\n");
            }
        }
    }

    /* Build join connections */
    v_iterate(joins, iter) {
        join = vl_iter_pval(iter);
        if (vh_iget(join, "NOPATH"))
            continue;

        oneway = vh_iget(join, "ONEWAY");
        len = vh_iget(join, "LEN");
        if (len)
            uselen++;

        from = vh_pget(join, "FROM");
        to = vh_pget(join, "TO");

        /* From -> to */
        reach = vh_create();

        cmd = NULL;
        goflag = vh_iget(join, "GO");
        if ((cmdto = vh_pget(join, "TO_CMD")) != NULL) {
            vh_pstore(reach, "CMD", cmdto);
        } else {
            if (goflag)
                cmd = dirinfo[goflag].sname;
            if (cmd == NULL)
                cmd = "?";
            add_attr(reach, "CMD", cmd);
        }

        vh_pstore(reach, "FROM", from);
        vh_pstore(reach, "TO", to);
        vh_pstore(reach, "NEED", vh_pget(join, "NEED"));
        vh_pstore(reach, "BEFORE", vh_pget(join, "BEFORE"));
        vh_pstore(reach, "AFTER", vh_pget(join, "AFTER"));
        vh_pstore(reach, "LEAVE", vh_pget(join, "LEAVE"));
        vh_istore(reach, "LEAVEALL", vh_iget(join, "LEAVEALL"));
        vh_istore(reach, "LEN", V_MAX(len, 1));

        link_rooms(from, to, reach);

        if (TASK_VERBOSE) {
            indent(1);
            list = vh_pget(reach, "CMD");
            printf("join '%s' to '%s' (%s)",
                   vh_sgetref(from, "DESC"),
                   vh_sgetref(to, "DESC"),
                   vl_join(list, ". "));
            if (len > 1)
                printf(" (dist %d)", len);
            printf("\n");
        }

        /* To -> from if not one-way */
        if (!oneway) {
            reach = vh_create();

            cmd = NULL;
            goflag = dirinfo[goflag].odir;
            if ((cmdfrom = vh_pget(join, "FROM_CMD")) != NULL) {
                vh_pstore(reach, "CMD", cmdfrom);
            } else if (cmdto != NULL) {
                vh_pstore(reach, "CMD", cmdto);
            } else {
                if (goflag)
                    cmd = dirinfo[goflag].sname;
                if (cmd == NULL)
                    cmd = "?";
                add_attr(reach, "CMD", cmd);
            }

            vh_pstore(reach, "FROM", to);
            vh_pstore(reach, "TO", from);
            vh_pstore(reach, "NEED", vh_pget(join, "NEED"));
            vh_pstore(reach, "BEFORE", vh_pget(join, "BEFORE"));
            vh_pstore(reach, "AFTER", vh_pget(join, "AFTER"));
            vh_pstore(reach, "LEAVE", vh_pget(join, "LEAVE"));
            vh_istore(reach, "LEAVEALL", vh_iget(join, "LEAVEALL"));
            vh_istore(reach, "LEN", V_MAX(len, 1));

            link_rooms(to, from, reach);

            if (TASK_VERBOSE) {
                indent(1);
                list = vh_pget(reach, "CMD");
                printf("join '%s' to '%s' (%s)",
                       vh_sgetref(to, "DESC"),
                       vh_sgetref(from, "DESC"),
                       vl_join(list, ". "));
                if (len > 1)
                    printf(" (dist %d)", len);
                printf("\n");
            }
        }
    }

    /* Set graph functions */
    vg_use_node_function(graph, use_node);
    vg_use_link_function(graph, use_link);

    if (uselen)
        vg_link_size_function(graph, link_size);
}

/* Return length of a path between two rooms */
int
find_path(vhash *step, vhash *from, vhash *to)
{
    vlist *path;
    int len;

    /* Check trivial case */
    if (from == to)
        return 0;

#ifdef SHOW_VISIT
    start_room = from;
#endif

    if (TASK_VERBOSE) {
        indent(3);
        printf("find path: '%s' to '%s'",
               vh_sgetref(from, "DESC"),
               vh_sgetref(to, "DESC"));
    }

    if (step != NULL && vh_iget(step, "BLOCK")) {
        if (TASK_VERBOSE)
            printf("\n");

        path_task = step;
        if ((path = vh_pget(step, "PATH")) != NULL)
            vl_destroy(path);
        vh_delete(step, "PATH");

        vg_use_cache(graph, 0);
        if ((path = PATH_INFO(from, to)) == NULL)
            return NOPATH;

        len = vl_ishift(path);
        vh_pstore(step, "PATH", path);
    } else {
        path_task = NULL;
        vg_use_cache(graph, 1);

        if (TASK_VERBOSE && path_room != from)
            printf("\n");

        len = PATH_LENGTH(from, to);

        if (TASK_VERBOSE && path_room == from) {
            if (len < 0)
                printf(" (cached: no path)\n");
            else
                printf(" (cached: dist %d)\n", len);
        }
    }

    return (len < 0 ? NOPATH : len);
}

/* Return path twixt path start room and a given room */
vlist *
get_path(vhash *step, vhash *room)
{
    static vlist *path = NULL;
    vlist *list, *rlist;
    char *node, *next;
    int i, len, found;
    vhash *reach;
    viter iter;

    /* Build path */
    list = vh_pget(step, "PATH");
    vh_delete(step, "PATH");

    if (list == NULL)
        list = PATH_NODES(path_room, room);

    if (list == NULL)
        return NULL;

    if (path == NULL)
        path = vl_create();
    else
        vl_empty(path);

    /* Record reach-elements that were used */
    len = vl_length(list);
    node = vl_sgetref(list, 0);

    for (i = 1; i < len; i++) {
        next = vl_sgetref(list, i);
        rlist = vg_link_pget(graph, node, next);
        found = 0;

        v_iterate(rlist, iter) {
            reach = vl_iter_pval(iter);
            if (!found && vh_iget(reach, "USE")) {
                vl_ppush(path, reach);
                found++;
            }
        }

        node = next;
    }

    vl_destroy(list);
    return path;
}

/* Initialise path searches */
void
init_path(vhash *room)
{
    vhash *step, *item, *taskroom;
    int len, blockable, offset;
    vlist *list;
    double dist;
    viter i, j;

    /* Only need update if room changed, or path modified */
    if (room == path_room && !path_modify)
        return;

    path_room = room;
    path_modify = 0;

    /* Deal with tasks that need droppable items */
    v_iterate(tasklist, i) {
        step = vl_iter_pval(i);
        if (vh_iget(step, "DONE"))
            continue;

        if ((list = vh_pget(step, "NEED")) == NULL)
            continue;

        if ((taskroom = vh_pget(step, "ROOM")) == NULL)
            continue;

        blockable = 0;
        v_iterate(list, j) {
            item = vl_iter_pval(j);

            if (!vh_iget(item, "TAKEN"))
                continue;

            if (!vh_iget(item, "LEAVE"))
                continue;

            vh_pstore(item, "BLOCK", step);
            blockable = 1;
        }

        if (blockable) {
            /* Find path to task room */
            vh_istore(step, "BLOCK", 1);

            if (TASK_VERBOSE) {
                indent(2);
                printf("update path: %s\n", vh_sgetref(step, "DESC"));
                indent(2);
                printf("possible block: %s may need dropping\n",
                       vh_sgetref(item, "DESC"));
            }

            if ((len = find_path(step, room, taskroom)) != NOPATH)
                vh_istore(step, "SORT", len);
            else
                vh_istore(step, "SORT", BIG);
        }
    }

    /* Cache paths from this room */
    solver_msg(2, "updating path cache");
    dist = vg_path_cache(graph, NODE(room));
    solver_msg(2, "updated path cache (max dist %g)", dist);

    /* Record distance of each task */
    v_iterate(tasklist, i) {
        step = vl_iter_pval(i);

        if (vh_iget(step, "BLOCK"))
            len = vh_iget(step, "SORT");
        else if ((taskroom = vh_pget(step, "ROOM")) == NULL)
            len = 0;
        else if ((len = PATH_LENGTH(room, taskroom)) < 0)
            len = BIG;

        /* Put 'get-item' tasks a bit further */
        offset = (vh_iget(step, "TYPE") == T_GET);
        vh_istore(step, "SORT", 2 * len + offset);
        vh_istore(step, "DIST", len);
    }

    /* Sort tasks according to distance */
    vl_sort(tasklist, sort_tasks);

    if (TASK_VERBOSE) {
        vhash *room;

        v_iterate(tasklist, i) {
            step = vl_iter_pval(i);

            if (vh_iget(step, "DONE") || vh_iget(step, "IGNORED"))
                continue;

            if ((len = vh_iget(step, "DIST")) == BIG)
                continue;

            if (require_task(step) != NULL)
                continue;

            indent(3);
            printf("dist %d: %s", len, vh_sgetref(step, "DESC"));
            if (len > 0 && (room = vh_pget(step, "ROOM")) != NULL)
                printf(" (%s)", vh_sgetref(room, "DESC"));
            printf("\n");
        }
    }
}

/* Link two rooms together */
static void
link_rooms(vhash *from, vhash *to, vhash *reach)
{
    char *fnode, *tnode;
    vlist *list;
    vhash *relt;
    viter iter;
    int len;

    fnode = vh_sgetref(from, "NODE");
    tnode = vh_sgetref(to, "NODE");

    /* Get list of reach-elements */
    if ((list = vg_link_pget(graph, fnode, tnode)) == NULL) {
        list = vl_create();
        vg_link_oneway_pstore(graph, fnode, tnode, list);
    }

    /* Check lengths agree (unfortunate limitation) */
    len = vh_iget(reach, "LEN");
    v_iterate(list, iter) {
        relt = vl_iter_pval(iter);
        if (len != vh_iget(relt, "LEN"))
            err("links between '%s' and '%s' have differing lengths",
                vh_sgetref(from, "DESC"), vh_sgetref(to, "DESC"));
    }

    /* Add new element */
    vl_ppush(list, reach);
}

/* Return length of a link */
static double
link_size(char *fnode, char *tnode, vscalar *s)
{
    vlist *list = vs_pget(s);
    vhash *reach = vl_pget(list, 0);
    return vh_dget(reach, "LEN");
}

/* Flag path modification */
void
modify_path(int print)
{
    path_modify = 1;

    if (print)
        solver_msg(2, "flag path cache update");
}

/* Return list of reachable rooms from a given room */
vlist *
reachable_rooms(vhash *room)
{
    static vlist *list = NULL;
    vlist *rlist, *tlist;
    char *node, *next;
    viter i, j;

    if (list == NULL)
        list = vl_create();
    else
        vl_empty(list);

    node = NODE(room);
    tlist = vg_node_to(graph, node);

    v_iterate(tlist, i) {
        next = vl_iter_svalref(i);
        rlist = vg_link_pget(graph, node, next);
        v_iterate(rlist, j)
            vl_ppush(list, vl_iter_pval(j));
    }

    vl_destroy(tlist);
    return list;
}

/* Task sorting function */
int
sort_tasks(vscalar **v1, vscalar **v2)
{
    vhash *t1 = vs_pget(*v1);
    vhash *t2 = vs_pget(*v2);
    int s1 = vh_iget(t1, "SORT");
    int s2 = vh_iget(t2, "SORT");

    /* Try sort codes first */
    if (s1 < s2)
        return -1;
    else if (s1 > s2)
        return 1;

    /* Try order of declaration */
    s1 = vh_iget(t1, "ID");
    s2 = vh_iget(t2, "ID");

    if (s1 < s2)
        return -1;
    else if (s1 > s2)
        return 1;

    /* Shouldn't get here */
    return 0;
}

/* Is link usable? */
static int
use_link(char *fnode, char *tnode, vscalar *s)
{
    vhash *reach, *item, *task, *tstep, *block, *room;
    vlist *list, *rlist;
    viter i, j;
    int flag;

    /* Loop over all reach elements of this link */
    rlist = vs_pget(s);
    v_iterate(rlist, i) {
        reach = vl_iter_pval(i);
        vh_istore(reach, "USE", 0);
        flag = 1;

        /* Check items needed by task don't have to be left */
        if (path_task != NULL && (list = vh_pget(reach, "LEAVE")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                block = vh_pget(item, "BLOCK");
                if (block != NULL && block == path_task) {
                    if (TASK_VERBOSE) {
                        indent(4 - vg_caching());
                        room = vg_node_pget(graph, tnode);
                        printf("blocked link: %s (must leave %s)\n",
                               vh_sgetref(room, "DESC"),
                               vh_sgetref(item, "DESC"));
                    }

                    flag = 0;
                    break;
                }
            }

            if (!flag)
                continue;
        }

        /* Check required items have been obtained */
        if ((list = vh_pget(reach, "NEED")) != NULL) {
            v_iterate(list, j) {
                item = vl_iter_pval(j);
                if (!vh_iget(item, "TAKEN")) {
                    if (TASK_VERBOSE) {
                        indent(4 - vg_caching());
                        room = vg_node_pget(graph, tnode);
                        printf("blocked link: %s (need %s)\n",
                               vh_sgetref(room, "DESC"),
                               vh_sgetref(item, "DESC"));
                    }

                    flag = 0;
                    break;
                }
            }

            if (!flag)
                continue;
        }

        /* Check status of required tasks */
        if ((list = vh_pget(reach, "BEFORE")) != NULL) {
            v_iterate(list, j) {
                task = vl_iter_pval(j);
                tstep = vh_pget(task, "STEP");
                if (vh_iget(tstep, "DONE")) {
                    if (TASK_VERBOSE) {
                        indent(4 - vg_caching());
                        room = vg_node_pget(graph, tnode);
                        printf("blocked link: %s (done '%s')\n",
                               vh_sgetref(room, "DESC"),
                               vh_sgetref(task, "DESC"));
                    }

                    flag = 0;
                    break;
                }
            }

            if (!flag)
                continue;
        }

        if ((list = vh_pget(reach, "AFTER")) != NULL) {
            v_iterate(list, j) {
                task = vl_iter_pval(j);
                tstep = vh_pget(task, "STEP");
                if (!vh_iget(tstep, "DONE")) {
                    if (TASK_VERBOSE) {
                        indent(4 - vg_caching());
                        room = vg_node_pget(graph, tnode);
                        printf("blocked link: %s (not done '%s')\n",
                               vh_sgetref(room, "DESC"),
                               vh_sgetref(task, "DESC"));
                    }

                    flag = 0;
                    break;
                }
            }

            if (!flag)
                continue;
        }

        /* Flag it as usable */
        vh_istore(reach, "USE", 1);
        return 1;
    }

    return 0;
}

/* Is room visitable? */
static int
use_node(char *node, vscalar *s, double dist)
{
    vhash *room, *item, *task, *tstep, *block;
    vlist *list;
    viter iter;

    room = vs_pget(s);

    /* Check items needed by task don't have to be left */
    if (path_task != NULL && (list = vh_pget(room, "LEAVE")) != NULL) {
        v_iterate(list, iter) {
            item = vl_iter_pval(iter);
            block = vh_pget(item, "BLOCK");
            if (block != NULL && block == path_task) {
                if (TASK_VERBOSE) {
                    indent(4 - vg_caching());
                    printf("blocked room: %s (must leave %s)\n",
                           vh_sgetref(room, "DESC"),
                           vh_sgetref(item, "DESC"));
                }

                return 0;
            }
        }
    }

    /* Check for required items */
    if ((list = vh_pget(room, "NEED")) != NULL) {
        v_iterate(list, iter) {
            item = vl_iter_pval(iter);
            if (!vh_iget(item, "TAKEN")) {
                if (TASK_VERBOSE) {
                    indent(4 - vg_caching());
                    printf("blocked room: %s (need %s)\n",
                           vh_sgetref(room, "DESC"),
                           vh_sgetref(item, "DESC"));
                }

                return 0;
            }
        }
    }

    /* Check status of required tasks */
    if ((list = vh_pget(room, "BEFORE")) != NULL) {
        v_iterate(list, iter) {
            task = vl_iter_pval(iter);
            tstep = vh_pget(task, "STEP");
            if (vh_iget(tstep, "DONE")) {
                if (TASK_VERBOSE) {
                    indent(4 - vg_caching());
                    printf("blocked room: %s (done '%s')\n",
                           vh_sgetref(room, "DESC"),
                           vh_sgetref(task, "DESC"));
                }

                return 0;
            }
        }
    }

    if ((list = vh_pget(room, "AFTER")) != NULL) {
        v_iterate(list, iter) {
            task = vl_iter_pval(iter);
            tstep = vh_pget(task, "STEP");
            if (!vh_iget(tstep, "DONE")) {
                if (TASK_VERBOSE) {
                    indent(4 - vg_caching());
                    printf("blocked room: %s (not done '%s')\n",
                           vh_sgetref(room, "DESC"),
                           vh_sgetref(task, "DESC"));
                }

                return 0;
            }
        }
    }

#ifdef SHOW_VISIT
    if (TASK_VERBOSE && room != start_room) {
        indent(4 - vg_caching());
        printf("visit: %s (dist %g)\n",
               vh_sgetref(room, "DESC"), dist);
    }
#endif

    return 1;
}
