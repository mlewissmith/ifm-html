/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Utility functions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <vars.h>

#include "ifm-driver.h"
#include "ifm-main.h"
#include "ifm-map.h"
#include "ifm-util.h"
#include "ifm-vars.h"

/* Direction info (same order as direction enum list) */
struct d_info dirinfo[] = {
    { "?",   "none",      D_NONE,      D_NONE,       0,  0 },
    { "N",   "north",     D_NORTH,     D_SOUTH,      0,  1 },
    { "S",   "south",     D_SOUTH,     D_NORTH,      0, -1 },
    { "E",   "east",      D_EAST,      D_WEST,       1,  0 },
    { "W",   "west",      D_WEST,      D_EAST,      -1,  0 },
    { "NE",  "northeast", D_NORTHEAST, D_SOUTHWEST,  1,  1 },
    { "SW",  "southwest", D_SOUTHWEST, D_NORTHEAST, -1, -1 },
    { "NW",  "northwest", D_NORTHWEST, D_SOUTHEAST, -1,  1 },
    { "SE",  "southeast", D_SOUTHEAST, D_NORTHWEST,  1, -1 },
    { "U",   "up",        D_UP,        D_DOWN,       0,  0 },
    { "D",   "down",      D_DOWN,      D_UP,         0,  0 },
    { "IN",  "in",        D_IN,        D_OUT,        0,  0 },
    { "OUT", "out",       D_OUT,       D_IN,         0,  0 },
    { NULL,  NULL,        D_NONE,      D_NONE,       0,  0 }
};

/* Standard paper sizes */
static struct paper_st {
    char *name;
    double width, height;
} paper_sizes[] = {
    { "A3",       29.7,   42.01 },
    { "A4",       21.0,   29.7  },
    { "A",        21.59,  27.94 },
    { "B",        27.94,  43.18 },
    { "C",        43.18,  55.88 },
    { "Legal",    21.59,  35.56 },
    { "Letter",   21.59,  27.94 },
    { NULL,       0.0,    0.0   }
};

/* Add a string attribute to an object */
void
add_attr(vhash *obj, char *attr, char *fmt, ...)
{
    vlist *list;
    V_BUF_DECL;
    char *str;

    list = vh_add_list(obj, attr);

    if (fmt != NULL) {
        V_BUF_FMT(fmt, str);
        vl_spush(list, str);
    } else {
        vl_empty(list);
    }
}

/* Add a object attribute to an object */
void
add_list(vhash *obj, char *attr, vhash *thing)
{
    vlist *list;

    list = vh_add_list(obj, attr);
    vl_ppush(list, thing);
}

/* Locate a file using the search path */
char *
find_file(char *name)
{
    V_BUF_DECL;
    char *path;
    viter iter;

    v_iterate(ifm_search, iter) {
        path = V_BUF_SET2("%s/%s", vl_iter_svalref(iter), name);
        if (v_exists(path))
            return path;
    }

    return NULL;
}

/* Return direction given offsets */
int
get_direction(int xoff, int yoff)
{
    int dir;

    for (dir = 0; dirinfo[dir].sname != NULL; dir++)
        if (xoff == dirinfo[dir].xoff && yoff == dirinfo[dir].yoff)
            return dir;

    return D_NONE;
}

/* Get page dimensions given a page description */
int
get_papersize(char *pagesize, float *width, float *height)
{
    int i;

    for (i = 0; paper_sizes[i].name != NULL; i++) {
        if (!strcasecmp(pagesize, paper_sizes[i].name)) {
            *width = paper_sizes[i].width;
            *height = paper_sizes[i].height;
            return 1;
        }
    }

    return 0;
}

/* Indent output line */
void
indent(int num)
{
    int i;

    for (i = 0; i < 4 * num; i++)
        putchar(' ');
}

/* Warn about obsolete syntax */
void
obsolete(char *old, char *new)
{
    static vhash *warned = NULL;

    if (warned == NULL)
        warned = vh_create();

    if (vh_exists(warned, old))
        return;

    vh_sstore(warned, old, new);
    warn("%s is obsolete -- use %s instead", old, new);
}

/* Pack sections onto virtual pages */
int
pack_sections(int xmax, int ymax)
{
    int pos, packed, x1, y1, x2, y2, xo, yo, num, xlen, ylen;
    int spacing = var_int("map_section_spacing");
    vlist *pages, *newpages, *psects, *opsects;
    double xo1, yo1, xo2, yo2, r1, r2, ratio;
    int v1, v2, rflag, xc1, yc1, xc2, yc2;
    vhash *page, *sect, *p1, *p2;
    viter i, j;

    /* Initialise -- one section per page */
    pages = vl_create();
    v_iterate(sects, i) {
        sect = vl_iter_pval(i);
        if (vh_iget(sect, "NOPRINT"))
            continue;

        vh_dstore(sect, "XOFF", 0.0);
        vh_dstore(sect, "YOFF", 0.0);

        xlen = vh_iget(sect, "XLEN");
        ylen = vh_iget(sect, "YLEN");

        page = vh_create();
        vh_istore(page, "XLEN", xlen);
        vh_istore(page, "YLEN", ylen);
        vl_ppush(pages, page);

        psects = vl_create();
        vh_pstore(page, "SECTS", psects);
        vl_ppush(psects, sect);
    }

    ratio = (double) xmax / ymax;

    /* Pack sections */
    do {
        pos = packed = 0;
        newpages = vl_create();

        while (pos < vl_length(pages)) {
            /* Get next page */
            p1 = vl_pget(pages, pos);
            x1 = vh_iget(p1, "XLEN");
            y1 = vh_iget(p1, "YLEN");

            /* Check if it's better off rotated */
            vh_istore(p1, "ROTATE", ((x1 < y1 && xmax > ymax) ||
                                     (x1 > y1 && xmax < ymax)));

            /* Check if this is the last page */
            if (pos + 1 == vl_length(pages)) {
                vl_ppush(newpages, p1);
                break;
            }

            /* Get following page */
            p2 = vl_pget(pages, pos + 1);
            x2 = vh_iget(p2, "XLEN");
            y2 = vh_iget(p2, "YLEN");

            /* Try combining pages in X direction */
            xc1 = x1 + x2 + spacing;
            yc1 = V_MAX(y1, y2);
            v1 = (xc1 <= xmax && yc1 <= ymax);
            r1 = (double) xc1 / yc1;

            /* Try combining pages in Y direction */
            xc2 = V_MAX(x1, x2);
            yc2 = y1 + y2 + spacing;
            v2 = (xc2 <= xmax && yc2 <= ymax);
            r2 = (double) xc2 / yc2;

            /* See which is best */
            if (v1 && v2) {
                if (V_ABS(ratio - r1) < V_ABS(ratio - r2))
                    v2 = 0;
                else
                    v1 = 0;
            }

            /* Just copy page if nothing can be done */
            if (!v1 && !v2) {
                vl_ppush(newpages, p1);
                pos++;
                continue;
            }

            /* Create merged page */
            page = vh_create();
            psects = vl_create();
            vh_pstore(page, "SECTS", psects);
            xo1 = yo1 = xo2 = yo2 = 0;

            if (v1) {
                vh_istore(page, "XLEN", xc1);
                vh_istore(page, "YLEN", yc1);
                xo2 = x1 + spacing;

                if (y1 < y2)
                    yo1 = (yc1 - y1) / 2;
                else
                    yo2 = (yc1 - y2) / 2;
            }

            if (v2) {
                vh_istore(page, "XLEN", xc2);
                vh_istore(page, "YLEN", yc2);
                yo1 = y2 + spacing;

                if (x1 < x2)
                    xo1 = (xc2 - x1) / 2;
                else
                    xo2 = (xc2 - x2) / 2;
            }

            /* Copy sections to new page, updating offsets */
            opsects = vh_pget(p1, "SECTS");
            v_iterate(opsects, j) {
                sect = vl_iter_pval(j);
                vl_ppush(psects, sect);

                xo = vh_iget(sect, "XOFF");
                vh_dstore(sect, "XOFF", xo + xo1);

                yo = vh_iget(sect, "YOFF");
                vh_dstore(sect, "YOFF", yo + yo1);
            }

            opsects = vh_pget(p2, "SECTS");
            v_iterate(opsects, j) {
                sect = vl_iter_pval(j);
                vl_ppush(psects, sect);

                xo = vh_iget(sect, "XOFF");
                vh_dstore(sect, "XOFF", xo + xo2);

                yo = vh_iget(sect, "YOFF");
                vh_dstore(sect, "YOFF", yo + yo2);
            }

            /* Get rid of old pages */
            vh_destroy(p1);
            vh_destroy(p2);

            /* Add merged page to list and go to next page pair */
            vl_ppush(newpages, page);
            pos += 2;
            packed++;
        }

        /* Switch to new page list */
        vl_destroy(pages);
        pages = newpages;
    } while (packed);

    /* Give each section its page info and clean up */
    num = 0;
    v_iterate(pages, i) {
        page = vl_iter_pval(i);

        psects = vh_pget(page, "SECTS");
        xlen = vh_iget(page, "XLEN");
        ylen = vh_iget(page, "YLEN");
        rflag = vh_iget(page, "ROTATE");

        num++;
        v_iterate(psects, j) {
            sect = vl_iter_pval(j);

            vh_istore(sect, "PAGE", num);
            vh_istore(sect, "PXLEN", xlen);
            vh_istore(sect, "PYLEN", ylen);
            vh_istore(sect, "ROTATE", rflag);
        }

        vh_destroy(page);
    }

    vl_destroy(pages);
    return num;
}

/* Send a string to stdout */
void
put_string(char *fmt, ...)
{
    V_BUF_DECL;
    char *str;

    V_BUF_FMT(fmt, str);
    fputs(str, stdout);
}

/* Set up room names */
void
setup_room_names(void)
{
    char tag[10], *name, *jstyle = var_string("join_format");
    vhash *room, *join, *from, *to;
    int jnum = 0;
    V_BUF_DECL;
    viter iter;

    /* Indicate joins if required */
    if (var_int("show_joins")) {
        v_iterate(joins, iter) {
            join = vl_iter_pval(iter);
            if (vh_iget(join, "HIDDEN"))
                continue;

            from = vh_pget(join, "FROM");
            to = vh_pget(join, "TO");

            if (*jstyle == 'n') {
                jnum++;
                sprintf(tag, " (%d)", jnum);
            } else {
                sprintf(tag, " (%c)", 'A' + jnum);
                jnum++;
            }

            name = vh_sgetref(from, "RDESC");
            if (strlen(name) == 0)
                name = vh_sgetref(from, "DESC");

            V_BUF_SET(name);
            V_BUF_ADD(tag);
            vh_sstore(from, "RDESC", V_BUF_VAL);

            name = vh_sgetref(to, "RDESC");
            if (strlen(name) == 0)
                name = vh_sgetref(to, "DESC");

            V_BUF_SET(name);
            V_BUF_ADD(tag);
            vh_sstore(to, "RDESC", V_BUF_VAL);
        }
    }

    /* Set room names of rooms that don't have one yet */
    v_iterate(rooms, iter) {
        room = vl_iter_pval(iter);
        if (!vh_exists(room, "RDESC"))
            vh_sstore(room, "RDESC", vh_sgetref(room, "DESC"));
    }

    /* Add tag names if required */
    if (show_tags) {
        v_iterate(rooms, iter) {
            room = vl_iter_pval(iter);
            if (vh_exists(room, "TAG")) {
                V_BUF_SET2("%s [%s]",
                           vh_sgetref(room, "RDESC"),
                           vh_sgetref(room, "TAG"));
                vh_sstore(room, "RDESC", V_BUF_VAL);
            }
        }
    }
}

/* Split a line based on a given width/height ratio */
vlist *
split_line(char *string, double ratio)
{
    int llen = (int) (sqrt((double) strlen(string)) * ratio);
    vlist *list = vl_create();
    char *tok, *val;
    V_BUF_DECL;

    tok = strtok(string, " \t");
    val = V_BUF_SET(tok);

    while ((tok = strtok(NULL, " \t")) != NULL) {
        if (strlen(val) + strlen(tok) + 1 > llen) {
            vl_spush(list, val);
            val = V_BUF_SET(tok);
        } else {
            val = V_BUF_ADD(" ");
            val = V_BUF_ADD(tok);
        }
    }

    vl_spush(list, val);
    return list;
}

/* Truncate a list of points based on a given box width and height */
void
truncate_points(vlist *x, vlist *y, double wid, double ht)
{
    double x1, y1, x2, y2, xf, yf, xl, yl;
    double xfac = 1 - wid / 2;
    double yfac = 1 - ht / 2;
    int np = vl_length(x);

    if (np < 2)
        return;

    /* Truncate first point */
    x1 = vl_dget(x, 1);
    y1 = vl_dget(y, 1);
    x2 = vl_dget(x, 0);
    y2 = vl_dget(y, 0);
    xf = x1 + xfac * (x2 - x1);
    yf = y1 + yfac * (y2 - y1);

    /* Truncate last point */
    x1 = vl_dget(x, np - 2);
    y1 = vl_dget(y, np - 2);
    x2 = vl_dget(x, np - 1);
    y2 = vl_dget(y, np - 1);
    xl = x1 + xfac * (x2 - x1);
    yl = y1 + yfac * (y2 - y1);

    /* Replace points */
    vl_dstore(x, 0, xf);
    vl_dstore(y, 0, yf);
    vl_dstore(x, np - 1, xl);
    vl_dstore(y, np - 1, yl);
}
