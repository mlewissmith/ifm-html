/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Recording output driver */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <vars.h>

#include "ifm-driver.h"
#include "ifm-map.h"
#include "ifm-util.h"
#include "ifm-vars.h"
#include "ifm-rec.h"

/* Task function list */
taskfuncs rec_taskfuncs = {
    NULL,
    rec_task_entry,
    NULL
};

/* Internal functions */
static void putline(char *str);

/* Task functions */
void
rec_task_entry(vhash *task)
{
    vlist *cmds;
    viter iter;

    if ((cmds = vh_pget(task, "CMD")) != NULL) {
        v_iterate(cmds, iter)
            putline(vl_iter_svalref(iter));
    } else {
        putline(vh_sgetref(task, "DESC"));
    }
}

static void
putline(char *str)
{
    while (*str != '\0')
        putchar(toupper(*str++));

    putchar('\n');
}
