/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Variable functions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <vars.h>

#include "ifm-main.h"
#include "ifm-util.h"
#include "ifm-vars.h"

#define INIT_VARS                                                       \
        if (nvars == NULL) cvars = nvars = vh_create();                 \
        if (styles == NULL) styles = vh_create();                       \
        if (rstyles == NULL) rstyles = vh_create()

#define VAR_CHECK(id, var)                                              \
        if ((var = var_get(id)) == NULL)                                \
            fatal("variable '%s' is not defined", id)

#define ALPHA(c) (c == '_' || isalpha(c))
#define ALNUM(c) (c == '_' || isalnum(c))

/* Current variables */
static vhash *cvars = NULL;

/* Defined non-style variables */
static vhash *nvars = NULL;

/* Defined styles */
static vhash *styles = NULL;

/* Referenced styles */
static vhash *rstyles = NULL;

/* Style list */
static vlist *style_list = NULL;

/* Internal functions */
static vhash *read_colour_defs(FILE *fp);
static char *var_encode(char *driver, char *var);
static void var_print(vhash *vars, char *style);

/* Add a style to the style list */
void
add_style(char *name)
{
    if (style_list == NULL)
        style_list = vl_create();

    vl_spush(style_list, name);
}

/* Return a copy of the current style list */
vlist *
current_styles(void)
{
    vlist *styles = NULL;

    if (ifm_styles != NULL)
        styles = vl_copy(ifm_styles);

    if (style_list != NULL) {
        if (styles == NULL)
            styles = vl_copy(style_list);
        else
            vl_append(styles, style_list);
    }

    return styles;
}

/* Load styles that have been referenced but not defined */
void
load_styles(void)
{
    vlist *list;
    V_BUF_DECL;
    char *name;
    viter iter;

    if (rstyles == NULL)
        return;

    list = vh_keys(rstyles);

    v_iterate(list, iter) {
        name = vl_iter_svalref(iter);
        if (vh_exists(styles, name))
            continue;

        V_BUF_SET1("%s.ifm", name);
        parse_input(V_BUF_VAL, 1, 0);

        if (!vh_exists(styles, name))
            warn("style '%s' referenced but not defined", name);
    }

    vl_destroy(list);
}

/* Push a style onto the style list */
void
push_style(char *name)
{
    debug("push style: %s", name);
    add_style(name);
    set_style(name);
}

/* Pop a style from the style list */
void
pop_style(char *name)
{
    char *sname;

    debug("pop style");

    if (style_list != NULL && vl_length(style_list) > 0) {
        sname = vl_spop(style_list);
        if (name != NULL && !V_STREQ(sname, name))
            warn("unexpected style: %s (expected %s)", name, sname);
        if (vl_length(style_list) > 0)
            set_style(vl_stail(style_list));
        else
            set_style(NULL);
    } else {
        warn("no matching style command");
    }
}

/* Read and return colour definitions from a stream */
static vhash *
read_colour_defs(FILE *fp)
{
    int red, green, blue, pos;
    char *line, name[100];
    vhash *defs;
    V_BUF_DECL;

    defs = vh_create();

    while ((line = V_BUF_FGETS(fp)) != NULL) {
        /* Get RGB values */
        V_BUF_CHOP;
        if (sscanf(line, "%d %d %d", &red, &green, &blue) != 3)
            continue;

        /* Get offset of colour name */
        if ((pos = strspn(line, "0123456789 \t\n")) != 0)
            strcpy(name, &line[pos]);
        else
            continue;

        /* Scale RGB values */
        V_BUF_SET3("%.3g %.3g %.3g",
                   red / 255.0, green / 255.0, blue / 255.0);

        /* Add colour */
        vh_sstore(defs, name, V_BUF_VAL);
    }

    return defs;
}

/* Mark a style as referenced */
void
ref_style(char *name)
{
    INIT_VARS;
    vh_istore(rstyles, name, 1);
}

/* Set the current style */
void
set_style(char *name)
{
    INIT_VARS;

    if (name != NULL && strlen(name) > 0)
        cvars = vh_add_hash(styles, name);
    else
        cvars = nvars;
}

/* Set the current style list */
void
set_style_list(vlist *list)
{
    style_list = list;
}

/* Return whether a variable has changed since last accessed */
int
var_changed(char *id)
{
    static vhash *values = NULL;
    vscalar *sval, *cval;
    int changed = 1;

    if (values == NULL)
        values = vh_create();

    sval = var_get(id);

    if ((cval = vh_get(values, id)) == NULL && sval == NULL)
        changed = 0;
    else if (cval != NULL && sval != NULL && vs_equal(cval, sval))
        changed = 0;

    if (changed)
        vh_store(values, id, vs_copy(sval));

    return changed;
}

/* Return RGB values from a variable */
char *
var_colour(char *id)
{
    static vhash *defs = NULL;
    char *name, *file, *path;
    double red, green, blue;
    FILE *fp;

    /* Get colour name */
    name = var_string(id);

    /* If colour name looks like RGB values, leave it */
    if (sscanf(name, "%lf %lf %lf", &red, &green, &blue) == 3)
        return name;

    /* Read colour definitions if required */
    if (defs == NULL) {
        file = var_string("colour_file");

        if ((path = find_file(file)) == NULL)
            fatal("can't locate colour definitions file '%s'", file);

        if ((fp = fopen(path, "r")) == NULL)
            fatal("can't open '%s'", path);

        defs = read_colour_defs(fp);
        fclose(fp);
    }

    /* Look up colour name */
    if (!vh_exists(defs, name))
        fatal("colour '%s' is not defined", name);

    return vh_sgetref(defs, name);
}

/* Encode a variable */
static char *
var_encode(char *driver, char *var)
{
    V_BUF_DECL;

    if (driver == NULL)
        driver = "global";

    if (strchr(var, '.') != NULL)
        V_BUF_SET(var);
    else
        V_BUF_SET2("%s.%s", driver, var);

    return V_BUF_VAL;
}

/* Return value of a variable */
vscalar *
var_get(char *id)
{
    char *key, *style;
    vscalar *var;
    vhash *svars;
    int i, len;

    INIT_VARS;

    /* Check style list if required */
    if (style_list != NULL) {
        len = vl_length(style_list);
        for (i = len - 1; i >= 0; i--) {
            style = vl_sgetref(style_list, i);
            if ((svars = vh_pget(styles, style)) == NULL)
                continue;

            /* Check current output format first */
            if (ifm_format != NULL) {
                key = var_encode(ifm_format, id);
                if ((var = vh_get(svars, key)) != NULL)
                    return var;
            }
        
            /* Check global variables */
            key = var_encode(NULL, id);
            if ((var = vh_get(svars, key)) != NULL)
                return var;
        }
    }

    /* Try non-style variables */
    if (ifm_format != NULL) {
        key = var_encode(ifm_format, id);
        if ((var = vh_get(nvars, key)) != NULL)
            return var;
    }
        
    key = var_encode(NULL, id);
    if ((var = vh_get(nvars, key)) != NULL)
        return var;

    return NULL;
}

/* Return an integer variable */
int
var_int(char *id)
{
    vscalar *var;
    VAR_CHECK(id, var);

    if (var == NULL)
        return 0;

    if (V_STREQ(vs_sget(var), "true"))
        return 1;

    if (V_STREQ(vs_sget(var), "false"))
        return 0;

    return vs_iget(var);
}

/* List variables to stdout */
void
var_list(void)
{
    vhash *svars;
    vlist *slist;
    char *style;
    viter iter;

    INIT_VARS;

    printf("# IFM defined variables.\n");

    /* Non-style variables */
    var_print(nvars, NULL);

    /* Style variables */
    slist = vh_keys(styles);

    v_iterate(slist, iter) {
        style = vl_iter_svalref(iter);
        svars = vh_pget(styles, style);
        if (svars != NULL)
            var_print(svars, style);
    }

    vl_destroy(slist);
}

/* Print out a set of variables */
static void
var_print(vhash *vars, char *style)
{
    char *name, *sval;
    vlist *names;
    vscalar *val;
    viter iter;

    names = vh_keys(vars);
    vl_sort(names, NULL);

    printf("\n");

    if (style != NULL)
        printf("# Style '%s' variables.\n", style);
    else
        printf("# General variables.\n");

    v_iterate(names, iter) {
        name = vl_iter_svalref(iter);
        printf("%s = ", name);
        val = vh_get(vars, name);

        switch (vs_type(val)) {

        case V_TYPE_STRING:
            sval = vs_sgetref(val);
            putchar('"');

            while (*sval != '\0') {
                if (*sval == '\\')
                    putchar('\\');
                putchar(*sval++);
            }

            putchar('"');
            break;

        default:
            printf("%s", vs_sget(val));
            break;
        }

        if (style != NULL)
            printf(" in style %s", style);

        printf(";\n");
    }

    vl_destroy(names);
}

/* Return a real variable */
double
var_real(char *id)
{
    vscalar *var;
    VAR_CHECK(id, var);
    return (var != NULL ? vs_dget(var) : 0.0);
}

/* Set a scalar variable */
void
var_set(char *driver, char *id, vscalar *val)
{
    char *key;

    INIT_VARS;

    key = var_encode(driver, id);

    if (val != NULL)
        vh_store(cvars, key, val);
    else
        vh_delete(cvars, key);
}

/* Return a string variable */
char *
var_string(char *id)
{
    vscalar *var;
    VAR_CHECK(id, var);
    return (var != NULL ? vs_sgetcopy(var) : "");
}
