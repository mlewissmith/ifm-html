/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* PostScript output driver */

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
#include "ifm-main.h"
#include "ifm-map.h"
#include "ifm-util.h"
#include "ifm-vars.h"
#include "ifm-ps.h"

#define PRINT_COLOUR(name) \
        if (var_changed(#name)) \
                printf("/%s [%s] def\n", #name, var_colour(#name))

#define PRINT_FONT(name) \
        if (var_changed(#name)) \
                printf("/%s /%s def\n", #name, var_string(#name))

#define PRINT_FONTSIZE(name) \
        if (var_changed(#name)) \
                printf("/%s %g def\n", #name, var_real(#name) * font_scale)

#define PRINT_INT(name) \
        if (var_changed(#name)) \
                printf("/%s %d def\n", #name, var_int(#name))

#define PRINT_REAL(name) \
        if (var_changed(#name)) \
                printf("/%s %g def\n", #name, var_real(#name))

#define PRINT_STRING(name) \
        if (var_changed(#name)) \
                printf("/%s %s def\n", #name, ps_string(var_string(#name)))

#define PRINT_BOOL(name) \
        if (var_changed(#name)) \
                printf("/%s %s def\n", #name, var_int(#name) ? "true" : "false")

/* Map function list */
mapfuncs ps_mapfuncs = {
    ps_map_start,
    ps_map_section,
    ps_map_room,
    ps_map_link,
    NULL,
    ps_map_endsection,
    ps_map_finish
};

/* Internal variables */
static int ps_rotate = 0;       /* Whether to rotate pages */
static int ps_rotflag = 0;      /* Whether to override auto-rotation */
static int ps_pagenum = 0;      /* Current page */

static double ps_xoff;          /* Current X offset */
static double ps_yoff;          /* Current Y offset */

/* Internal functions */
static void ps_print_room_vars(void);
static void ps_print_link_vars(void);
static char *ps_string(char *str);

/* Map functions */
void
ps_map_start(void)
{
    int ylen, c, num_pages, width, height;
    char *title, *prolog, *file;
    vhash *sect;
    viter iter;
    FILE *fp;

    /* Locate prolog file */
    file = var_string("prolog_file");
    if ((prolog = find_file(file)) == NULL)
        fatal("can't find PostScript prolog '%s'", file);

    /* Allow title space for sections with titles */
    v_iterate(sects, iter) {
        sect = vl_iter_pval(iter);
        if (show_map_title && vh_exists(sect, "TITLE")) {
            ylen = vh_iget(sect, "YLEN");
            vh_istore(sect, "YLEN", ylen + 1);
        }
    }

    /* Set room names */
    setup_room_names();

    /* Check overriding of page rotation */
    ps_rotflag = VAR_DEF("page_rotate");
    if (ps_rotflag)
        ps_rotate = var_int("page_rotate");

    /* Get desired dimensions, in rooms */
    width = (int) (page_width / room_size) + 1;
    height = (int) (page_height / room_size) + 1;

    /* Pack sections */
    num_pages = pack_sections(width, height);

    /* Print header */
    if (vh_exists(map, "TITLE"))
        title = vh_sgetref(map, "TITLE");
    else
        title = "Interactive Fiction map";

    printf("%%!PS-Adobe-2.0\n");
    put_string("%%%%Title: %s\n", title);
    printf("%%%%Creator: IFM v%s\n", VERSION);
    printf("%%%%Pages: %d\n", num_pages);
    printf("%%%%EndComments\n\n");

    /* Print PostScript prolog */
    if ((fp = fopen(prolog, "r")) == NULL)
        fatal("can't open '%s'", prolog);

    while ((c = fgetc(fp)) != EOF)
        putchar(c);

    fclose(fp);

    /* Page variables */
    printf("/page_margin %g cm def\n", var_real("page_margin"));
    printf("/page_width %g cm def\n", page_width);
    printf("/page_height %g cm def\n", page_height);
    printf("/map_width %d def\n", width);
    printf("/map_height %d def\n", height);

    PRINT_BOOL(show_page_border);
    PRINT_COLOUR(page_border_colour);
    PRINT_COLOUR(page_background_colour);

    /* Title variables */
    if (title != NULL) {
        PRINT_BOOL(show_page_title);
        put_string("/titlestring %s def\n", ps_string(title));
        PRINT_FONT(page_title_font);
        PRINT_FONTSIZE(page_title_fontsize);
        PRINT_COLOUR(page_title_colour);
    } else {
        printf("/show_page_title false def\n");
    }

    /* Map variables */
    PRINT_FONT(map_title_font);
    PRINT_FONTSIZE(map_title_fontsize);
    PRINT_COLOUR(map_title_colour);
    PRINT_COLOUR(map_border_colour);
    PRINT_COLOUR(map_background_colour);

    printf("/room_width %g def\n", room_width);
    printf("/room_height %g def\n", room_height);

    /* Room style variables */
    ps_print_room_vars();

    /* Link style variables */
    ps_print_link_vars();

    printf("\n%%%%EndProlog\n");
}

void
ps_map_section(vhash *sect)
{
    int page, xlen, ylen, rotate;
    double xpos, ypos;

    /* Get section parameters */
    page = vh_iget(sect, "PAGE");
    ps_xoff = vh_dget(sect, "XOFF");
    ps_yoff = vh_dget(sect, "YOFF");
    xlen = vh_iget(sect, "XLEN");
    ylen = vh_iget(sect, "YLEN");

    /* Start a new page if required */
    if (page != ps_pagenum) {
        if (ps_pagenum > 0)
            printf("endpage\n");

        ps_pagenum = page;
        printf("\n%%%%Page: %d %d\n\n", ps_pagenum, ps_pagenum);

        rotate = (ps_rotflag ? ps_rotate : vh_iget(sect, "ROTATE"));

        printf("%d %d %s beginpage\n",
               vh_iget(sect, "PXLEN"),
               vh_iget(sect, "PYLEN"),
               (rotate ? "true" : "false"));
    }

    /* Print border if required */
    if (show_map_border)
        printf("%g %g %g %g mapborder\n",
               ps_xoff - 0.5, ps_yoff - 0.5,
               ps_xoff + xlen - 0.5, ps_yoff + ylen - 0.5);

    /* Print title if required */
    if (show_map_title && vh_exists(sect, "TITLE")) {
        xpos = (double) (xlen - 1) / 2;
        ypos = (double) ylen - 1;
        put_string("%s %g %g maptitle\n",
                   ps_string(vh_sgetref(sect, "TITLE")),
                   xpos + ps_xoff, ypos + ps_yoff);
    }
}

void
ps_map_room(vhash *room)
{
    static vlist *px = NULL, *py = NULL;
    vlist *items, *ex, *ey;
    char *itemlist = NULL;
    int x, y;

    /* Room style variables */
    ps_print_room_vars();

    /* Write coords */
    x = vh_iget(room, "X");
    y = vh_iget(room, "Y");
    put_string("%s %g %g",
               ps_string(vh_sgetref(room, "RDESC")),
               x + ps_xoff, y + ps_yoff);

    /* Write item list (if any) */
    items = vh_pget(room, "ITEMS");
    if (items != NULL && vl_length(items) > 0) {
        vhash *item;
        vlist *list;
        viter iter;

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

    if (itemlist != NULL)
        put_string(" %s true", ps_string(itemlist));
    else
        printf(" false");

    printf(" room\n");

    /* Write room exits (if any) */
    ex = vh_pget(room, "EX");
    ey = vh_pget(room, "EY");
    if (ex != NULL) {
        double x1, y1, x2, y2;

        if (px == NULL) {
            px = vl_create();
            py = vl_create();
        }

        while (vl_length(ex) > 0) {
            vl_istore(px, 0, x);
            vl_istore(py, 0, y);
            vl_istore(px, 1, x + vl_ishift(ex));
            vl_istore(py, 1, y + vl_ishift(ey));
            truncate_points(px, py, room_width, room_height);
            x1 = vl_dget(px, 0);
            y1 = vl_dget(py, 0);
            x2 = vl_dget(px, 1);
            y2 = vl_dget(py, 1);

            x2 = x1 + 0.35 * (x2 - x1);
            y2 = y1 + 0.35 * (y2 - y1);

            printf("%g %g %g %g roomexit\n",
                   ps_xoff + x1, ps_yoff + y1,
                   ps_xoff + x2, ps_yoff + y2);
        }
    }
}

void
ps_map_link(vhash *link)
{
    int oneway = vh_iget(link, "ONEWAY");
    int go = vh_iget(link, "GO");
    int up = (go == D_UP) ? 1 : (go == D_DOWN) ? -1 : 0;
    int in = (go == D_IN) ? 1 : (go == D_OUT) ? -1 : 0;
    vlist *x, *y;
    int i, np;

    /* Link style variables */
    ps_print_link_vars();

    x = vh_pget(link, "X");
    y = vh_pget(link, "Y");
    truncate_points(x, y, room_width, room_height);

    /*
     * Check for circular links.  A circular link is a two-segment link
     * that ends where it begins.  The midpoint indicates the exit
     * direction.
     */
    if ((vl_length(x) == 3) &&
	(vl_dget(x, 0) == vl_dget(x, 2)) &&
	(vl_dget(y, 0) == vl_dget(y, 2))) {
        double xs, ys, xm, ym;
        int angle;

        xs = vl_dget(x, 0);
        ys = vl_dget(y, 0);
        xm = vl_dget(x, 1);
        ym = vl_dget(y, 1);

        if (xm > xs) {
            if (ym < ys)
                angle = 315;
            else if (ym == ys)
                angle = 0;
            else
                angle = 45;
        } else if (xm < xs) {
            if (ym < ys)
                angle = 225;
            else if (ym == ys)
                angle = 180;
            else
                angle = 135;
        } else {
            if (ym < ys)
                angle = 270;
            else /* ym > ys */
                angle = 90;
        }

        printf("%g %g %d circle\n", xs + ps_xoff, ys + ps_yoff, angle);
    } else { /* it is not a circular link */
        printf("[");
        np = vl_length(x);
        for (i = 0; i < np; i++)
            printf(" %g %g",
                   vl_dget(x, i) + ps_xoff,
                   vl_dget(y, i) + ps_yoff);
        printf(" ]");

        printf(" %d", up);
        printf(" %d", in);
        printf(" %s", (oneway ? "true" : "false"));

        printf(" link\n");
    }
}

void
ps_map_endsection(void)
{
    /* Nothing to do */
}

void
ps_map_finish(void)
{
    printf("endpage\n");
}

/* Print room style variables */
static void
ps_print_room_vars(void)
{
    PRINT_FONT(room_text_font);
    PRINT_FONTSIZE(room_text_fontsize);
    PRINT_COLOUR(room_colour);
    PRINT_COLOUR(room_text_colour);
    PRINT_COLOUR(room_border_colour);
    PRINT_BOOL(room_border_dashed);
    PRINT_REAL(room_border_width);
    PRINT_REAL(room_shadow_xoff);
    PRINT_REAL(room_shadow_yoff);
    PRINT_COLOUR(room_shadow_colour);
    PRINT_COLOUR(room_exit_colour);
    PRINT_REAL(room_exit_width);
    PRINT_BOOL(show_items);
    PRINT_FONT(item_text_font);
    PRINT_FONTSIZE(item_text_fontsize);
    PRINT_COLOUR(item_text_colour);
}

/* Print room style variables */
static void
ps_print_link_vars(void)
{
    PRINT_COLOUR(link_colour);
    PRINT_BOOL(link_dashed);
    PRINT_REAL(link_arrow_size);
    PRINT_FONT(link_text_font);
    PRINT_FONTSIZE(link_text_fontsize);
    PRINT_COLOUR(link_text_colour);
    PRINT_REAL(link_line_width);
    PRINT_STRING(link_updown_string);
    PRINT_STRING(link_inout_string);
}

/* Return a string suitable for passing to PostScript */
static char *
ps_string(char *str)
{
    static vbuffer *b = NULL;

    if (b == NULL)
        b = vb_create();
    else
        vb_empty(b);

    vb_putc(b, '(');

    while (*str != '\0') {
        if (strchr("()", *str) != NULL)
            vb_putc(b, '\\');
        vb_putc(b, *str++);
    }

    vb_putc(b, ')');
    return vb_get(b);
}
