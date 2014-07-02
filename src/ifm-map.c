/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Functions for building the map */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vars.h>

#include "ifm-driver.h"
#include "ifm-main.h"
#include "ifm-map.h"
#include "ifm-util.h"

#define WARN_CROSS(room, from, to) \
        warn("room '%s' crossed by link line between '%s' and '%s'", \
             vh_sgetref(room, "DESC"), \
             vh_sgetref(from, "DESC"), \
             vh_sgetref(to, "DESC"))

vhash *map = NULL;              /* The map */

vlist *rooms = NULL;            /* List of rooms */
vlist *links = NULL;            /* List of links */
vlist *joins = NULL;            /* List of joins */
vlist *items = NULL;            /* List of items */
vlist *tasks = NULL;            /* List of tasks */
vlist *sects = NULL;            /* List of sections */

vhash *roomtags = NULL;         /* Tag -> room mapping */
vhash *itemtags = NULL;         /* Tag -> item mapping */
vhash *linktags = NULL;         /* Tag -> link mapping */
vhash *jointags = NULL;         /* Tag -> join mapping */
vhash *tasktags = NULL;         /* Tag -> task mapping */

vhash *startroom = NULL;        /* Starting room */

vlist *taskorder = NULL;        /* Ordered task list */
vlist *sectnames = NULL;	/* List of section names */

/* Internal stuff */
static vhash *rpos = NULL;      /* Room positions */

/* Internal functions */
static void put_room_at(vhash *room, int sect, int x, int y);
static vhash *room_at(int sect, int x, int y);
static void resolve_tag(char *type, vscalar *elt, vhash *table);
static void resolve_tag_list(char *type, vlist *list, vhash *table);

/* Initialise the map */
void
init_map(void)
{
    map = vh_create();
    rooms = vl_create();
    links = vl_create();
    items = vl_create();
    joins = vl_create();
    tasks = vl_create();
    sects = vl_create();

    rpos = vh_create();

    roomtags = vh_create();
    itemtags = vh_create();
    linktags = vh_create();
    jointags = vh_create();
    tasktags = vh_create();

    taskorder = vl_create();

    vh_pstore(map, "ROOMS", rooms);
    vh_pstore(map, "LINKS", links);
    vh_pstore(map, "ITEMS", items);
    vh_pstore(map, "JOINS", joins);
    vh_pstore(map, "TASKS", taskorder);
    vh_pstore(map, "SECTS", sects);

    vh_pstore(map, "RPOS", rpos);

    vh_pstore(map, "ROOMTAGS", roomtags);
    vh_pstore(map, "ITEMTAGS", itemtags);
    vh_pstore(map, "TASKTAGS", tasktags);
}

/* Put room at a given location */
static void
put_room_at(vhash * room, int sect, int x, int y)
{
    V_BUF_DECL;

    V_BUF_SET3("%d,%d,%d", sect, x, y);
    vh_pstore(rpos, V_BUF_VAL, room);
    vh_istore(room, "X", x);
    vh_istore(room, "Y", y);
}

/* Resolve a tag */
static void
resolve_tag(char *type, vscalar *elt, vhash *table)
{
    vhash *hash;
    char *tag;

    if (elt == NULL)
        return;

    if (vs_type(elt) != V_TYPE_STRING)
        return;

    tag = vs_sgetref(elt);
    hash = vh_pget(table, tag);

    if (hash != NULL) {
        vs_pstore(elt, hash);
    } else {
        err("%s tag '%s' not defined", type, tag);
        vh_pstore(table, tag, table);
    }
}

/* Resolve a tag list */
static void
resolve_tag_list(char *type, vlist *list, vhash *table)
{
    viter iter;

    if (list == NULL)
        return;

    v_iterate(list, iter)
        resolve_tag(type, vl_iter_val(iter), table);
}

/* Resolve all tags */
void
resolve_tags(void)
{
    vhash *room, *item, *link, *join, *task;
    vlist *list;
    viter iter;

    /* Resolve room tags */
    v_iterate(rooms, iter) {
        room = vl_iter_pval(iter);

        resolve_tag_list("task", vh_pget(room, "BEFORE"), tasktags);
        resolve_tag_list("task", vh_pget(room, "AFTER"), tasktags);
        resolve_tag_list("item", vh_pget(room, "NEED"), itemtags);
        resolve_tag_list("item", vh_pget(room, "LEAVE"), itemtags);
        resolve_tag_list("task", vh_pget(room, "LINK_BEFORE"), tasktags);
        resolve_tag_list("task", vh_pget(room, "LINK_AFTER"), tasktags);
        resolve_tag_list("item", vh_pget(room, "LINK_NEED"), itemtags);
        resolve_tag_list("item", vh_pget(room, "LINK_LEAVE"), itemtags);
    }

    /* Resolve link tags */
    v_iterate(links, iter) {
        link = vl_iter_pval(iter);

        resolve_tag("room", vh_get(link, "FROM"), roomtags);
        resolve_tag("room", vh_get(link, "TO"), roomtags);
        resolve_tag_list("task", vh_pget(link, "BEFORE"), tasktags);
        resolve_tag_list("task", vh_pget(link, "AFTER"), tasktags);
        resolve_tag_list("item", vh_pget(link, "NEED"), itemtags);
        resolve_tag_list("item", vh_pget(link, "LEAVE"), itemtags);
    }

    /* Resolve join tags */
    v_iterate(joins, iter) {
        join = vl_iter_pval(iter);

        resolve_tag("room", vh_get(join, "FROM"), roomtags);
        resolve_tag("room", vh_get(join, "TO"), roomtags);
        resolve_tag_list("task", vh_pget(join, "BEFORE"), tasktags);
        resolve_tag_list("task", vh_pget(join, "AFTER"), tasktags);
        resolve_tag_list("item", vh_pget(join, "NEED"), itemtags);
        resolve_tag_list("item", vh_pget(join, "LEAVE"), itemtags);
    }

    /* Resolve item tags and build room item lists */
    v_iterate(items, iter) {
        item = vl_iter_pval(iter);

        resolve_tag("room", vh_get(item, "IN"), roomtags);
        resolve_tag_list("task", vh_pget(item, "BEFORE"), tasktags);
        resolve_tag_list("task", vh_pget(item, "AFTER"), tasktags);
        resolve_tag_list("item", vh_pget(item, "NEED"), itemtags);
        resolve_tag_list("item", vh_pget(item, "KEEP_WITH"), itemtags);
        resolve_tag_list("task", vh_pget(item, "KEEP_UNTIL"), tasktags);

        room = vh_pget(item, "IN");

        if (room != NULL) {
            vh_pstore(item, "ROOM", room);
            list = vh_add_list(room, "ITEMS");
            vl_ppush(list, item);
        }
    }

    /* Resolve task tags */
    v_iterate(tasks, iter) {
        task = vl_iter_pval(iter);

        resolve_tag("room", vh_get(task, "IN"), roomtags);
        resolve_tag("room", vh_get(task, "GOTO"), roomtags);
        resolve_tag("task", vh_get(task, "FOLLOW"), tasktags);
        resolve_tag("room", vh_get(task, "DROPROOM"), roomtags);

        resolve_tag_list("task", vh_pget(task, "AFTER"), tasktags);
        resolve_tag_list("task", vh_pget(task, "DO"), tasktags);
        resolve_tag_list("item", vh_pget(task, "NEED"), itemtags);
        resolve_tag_list("item", vh_pget(task, "GET"), itemtags);
        resolve_tag_list("item", vh_pget(task, "GIVE"), itemtags);
        resolve_tag_list("item", vh_pget(task, "LOSE"), itemtags);
        resolve_tag_list("item", vh_pget(task, "DROP"), itemtags);
        resolve_tag_list("task", vh_pget(task, "DROPUNTIL"), tasktags);

        vh_pstore(task, "ROOM", vh_pget(task, "IN"));
    }
}

/* Return room at given location */
static vhash *
room_at(int sect, int x, int y)
{
    V_BUF_DECL;

    V_BUF_SET3("%d,%d,%d", sect, x, y);
    return vh_pget(rpos, V_BUF_VAL);
}

/* Set/unset a room exit */
void
room_exit(vhash *room, int xoff, int yoff, int flag)
{
    vhash *flags;
    V_BUF_DECL;
    int num;

    if (xoff == 0 && yoff == 0)
        fatal("internal: invalid direction offset");

    flags = vh_add_hash(room, "EXIT");
    V_BUF_SET2("%d,%d", xoff, yoff);
    num = vh_iget(flags, V_BUF_VAL);
    num = V_MAX(num, 0);

    vh_istore(flags, V_BUF_VAL, (flag ? -1 : num + 1));
}

/* Set a tag table entry */
void
set_tag(char *type, char *tag, vhash *val, vhash *table)
{
    if (vh_pget(table, tag) == NULL) {
	vh_pstore(table, tag, val);
	vh_sstore(val, "TAG", tag);
    } else {
	err("%s tag '%s' already defined", type, tag);
    }
}

/* Set up room exits */
void
setup_exits(void)
{
    vhash *room, *flags;
    int x, y, num, dir;
    vlist *ex, *ey;
    viter i, j;
    char *tag;

    v_iterate(rooms, i) {
        room = vl_iter_pval(i);
        flags = vh_pget(room, "EXIT");
        if (flags == NULL)
            continue;

        ex = ey = NULL;
        v_iterate(flags, j) {
            tag = vh_iter_key(j);
            num = vh_iter_ival(j);

            if (num < 0) {
                if (ex == NULL) {
                    ex = vl_create();
                    ey = vl_create();
                    vh_pstore(room, "EX", ex);
                    vh_pstore(room, "EY", ey);
                }

                sscanf(tag, "%d,%d", &x, &y);
                vl_ipush(ex, x);
                vl_ipush(ey, y);
            } else if (num > 1) {
                sscanf(tag, "%d,%d", &x, &y);
                dir = get_direction(x, y);
                warn("room '%s' has multiple %s links",
                     vh_sgetref(room, "DESC"),
                     dirinfo[dir].lname);
            }
        }

        vh_destroy(flags);
        vh_delete(room, "EXIT");
    }
}

/* Set up room links */
void
setup_links(void)
{
    int x, y, xt, yt, xoff, yoff, dir, num, count, ndirs;
    vhash *link, *from, *to, *other, *sect;
    vlist *dirs, *xpos, *ypos, *list;
    char *fname, *tname;
    viter i, j;

    /* Build coordinate list for each link */
    v_iterate(links, i) {
	link = vl_iter_pval(i);

/*         if (vh_iget(link, "HIDDEN")) */
/*             continue; */

        if (vh_iget(link, "NOLINK"))
            continue;

	from = vh_pget(link, "FROM");
	fname = vh_sgetref(from, "DESC");

	to = vh_pget(link, "TO");
	tname = vh_sgetref(to, "DESC");

	sect = vh_pget(from, "SECT");
        num = vh_iget(sect, "NUM");

        /* Check rooms are linkable */
	if (vh_pget(to, "SECT") != sect) {
	    err("can't link '%s' to '%s' -- different map sections",
                fname, tname);
	    continue;
	}

        if (from == to && vh_pget(link, "DIR") == NULL) {
            err("can't link '%s' to itself without at least one direction",
                fname);
            continue;
        }

        /* Add link to section links */
        list = vh_pget(sect, "LINKS");
        vl_ppush(list, link);
        vh_pstore(link, "SECT", sect);

        /* Get from/to locations */
	x = vh_iget(from, "X");
	y = vh_iget(from, "Y");
	xt = vh_iget(to, "X");
	yt = vh_iget(to, "Y");

	/*
	 * Our canonical representation of a circular link is a two-segment
	 * link that goes out one way and comes right back in the same
	 * direction (recognizable by its equal start and endpoints, with
	 * the midpoint indicating the exit direction).  Transform all
	 * circular, one-way links into this sort of link by removing all
	 * but the first direction in the list.  (We don't mess with
	 * two-way links, because they have two exits, not one, and we
	 * don't want to alter that.)
	 */
	if (vh_iget(link, "ONEWAY") && (x == xt) && (y == yt)) {
            dirs = vh_pget(link, "DIR");
            while (vl_length(dirs) > 1)
                vl_ipop(dirs);
	}

        /* Initialise coordinate lists */
	xpos = vl_create();
	vh_pstore(link, "X", xpos);

	ypos = vl_create();
	vh_pstore(link, "Y", ypos);

        vl_ipush(xpos, x);
        vl_ipush(ypos, y);
        count = 0;

        if (vh_iget(link, "HIDDEN"))
            continue;

        /* Traverse direction list */
        if ((dirs = vh_pget(link, "DIR")) != NULL) {
            ndirs = vl_length(dirs);

            v_iterate(dirs, j) {
                dir = vl_iter_ival(j);

                xoff = dirinfo[dir].xoff;
                yoff = dirinfo[dir].yoff;

                /* Move to new coordinates */
                x += xoff;
                y += yoff;
                vl_ipush(xpos, x);
                vl_ipush(ypos, y);

                /* Check for first direction */
                if (count++ == 0) {
                    /* Remove any room exits */
                    room_exit(from, xoff, yoff, 0);

                    /* Record 'to' direction */
                    vh_istore(link, "TO_DIR", dir);
                }

                /* Check for crossing other rooms */
                if ((x != xt || y != yt || count < ndirs)
                    && (other = room_at(num, x, y)) != NULL)
                    WARN_CROSS(other, from, to);
            }
        }

        if (x == xt && y == yt) {
            /* Already at target room */
            room_exit(to, -xoff, -yoff, 0);
        } else {
            /* Get final direction to target room */
            xoff = xt - x;
            yoff = yt - y;

            /* Check it's on the grid */
            if (xoff != 0 && yoff != 0 && abs(xoff) != abs(yoff)) {
                warn("link from '%s' to '%s' outside grid",
                     fname, tname);
            } else {
                /* Unitize direction */
                if (xoff != 0)
                    xoff = (xoff > 0 ? 1 : -1);

                if (yoff != 0)
                    yoff = (yoff > 0 ? 1 : -1);

                /* Remove room exits */
                if (count == 0)
                    room_exit(from, xoff, yoff, 0);

                room_exit(to, -xoff, -yoff, 0);

                /* Add final coordinates */
                while (1) {
                    x += xoff;
                    y += yoff;

                    if (x == xt && y == yt)
                        break;

                    vl_ipush(xpos, x);
                    vl_ipush(ypos, y);

                    if ((other = room_at(num, x, y)) != NULL)
                        WARN_CROSS(other, from, to);
                }
            }

            vl_ipush(xpos, xt);
            vl_ipush(ypos, yt);
        }

        /* Record 'from' direction */
        vh_istore(link, "FROM_DIR", get_direction(-xoff, -yoff));

        /* Record 'to' direction if not already done */
        if (count == 0)
            vh_istore(link, "TO_DIR", get_direction(xoff, yoff));
    }
}

/* Set up room positions */
void
setup_rooms(void)
{
    vhash *base, *room, *link, *near, *other, *sect;
    int nfound, x, y, dir, num;
    vlist *list, *dirs;
    viter i, j, k;

    v_iterate(sects, i) {
        sect = vl_iter_pval(i);

        num = vh_iget(sect, "NUM");
        list = vh_pget(sect, "ROOMS");

	/* Position base room */
	base = vl_ptail(list);
	put_room_at(base, num, 0, 0);

	/* Position all other rooms relative to it */
        do {
	    nfound = 0;

	    v_iterate(list, j) {
		room = vl_iter_pval(j);

                if (vh_exists(room, "X"))
		    continue;

		if ((link = vh_pget(room, "LINK")) == NULL)
                    continue;

                near = vh_pget(link, "FROM");
                if (!vh_exists(near, "X"))
		    continue;

		x = vh_iget(near, "X");
		y = vh_iget(near, "Y");

		dirs = vh_pget(link, "DIR");
		v_iterate(dirs, k) {
		    dir = vl_iter_ival(k);
		    x += dirinfo[dir].xoff;
		    y += dirinfo[dir].yoff;
		}

		if ((other = room_at(num, x, y)) != NULL)
		    warn("rooms '%s' and '%s' overlap",
                         vh_sgetref(room, "DESC"),
                         vh_sgetref(other, "DESC"));

		put_room_at(room, num, x, y);
		nfound++;
	    }
	} while (nfound > 0);
    }
}

/* Set up sections */
void
setup_sections(void)
{
    int minx, miny, maxx, maxy, x, y, first, pos;
    vhash *sect, *room, *link;
    vlist *list, *xpos, *ypos;
    viter i, j, k;
    char *title;

    v_iterate(sects, i) {
        sect = vl_iter_pval(i);

        /* Set title */
        if (sectnames != NULL && vl_length(sectnames) > 0) {
            title = vl_sshift(sectnames);
            vh_sstore(sect, "TITLE", title);
        }

        /* Find width and length of section */
        first = 1;
        list = vh_pget(sect, "ROOMS");

        v_iterate(list, j) {
            room = vl_iter_pval(j);

            x = vh_iget(room, "X");
            y = vh_iget(room, "Y");

            if (first) {
                minx = maxx = x;
                miny = maxy = y;
                first = 0;
            } else {
                minx = V_MIN(minx, x);
                maxx = V_MAX(maxx, x);
                miny = V_MIN(miny, y);
                maxy = V_MAX(maxy, y);
            }
        }

        list = vh_pget(sect, "LINKS");

        v_iterate(list, j) {
            link = vl_iter_pval(j);

            xpos = vh_pget(link, "X");
            v_iterate(xpos, k) {
                x = vl_iter_ival(k);
                minx = V_MIN(minx, x);
                maxx = V_MAX(maxx, x);
            }

            ypos = vh_pget(link, "Y");
            v_iterate(ypos, k) {
                y = vl_iter_ival(k);
                miny = V_MIN(miny, y);
                maxy = V_MAX(maxy, y);
            }
        }

        vh_istore(sect, "XLEN", maxx - minx + 1);
        vh_istore(sect, "YLEN", maxy - miny + 1);

        /* Normalize all coordinates */
        list = vh_pget(sect, "ROOMS");

        v_iterate(list, j) {
            room = vl_iter_pval(j);
            x = vh_iget(room, "X");
            y = vh_iget(room, "Y");
            vh_istore(room, "X", x - minx);
            vh_istore(room, "Y", y - miny);
        }

        list = vh_pget(sect, "LINKS");

        v_iterate(list, j) {
            link = vl_iter_pval(j);

            xpos = vh_pget(link, "X");
            for (pos = 0; pos < vl_length(xpos); pos++) {
                x = vl_iget(xpos, pos);
                vl_istore(xpos, pos, x - minx);
            }

            ypos = vh_pget(link, "Y");
            for (pos = 0; pos < vl_length(ypos); pos++) {
                y = vl_iget(ypos, pos);
                vl_istore(ypos, pos, y - miny);
            }
        }
    }
}
