/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Text output driver header */

#ifndef IFM_TEXT_H
#define IFM_TEXT_H

extern itemfuncs text_itemfuncs;
extern taskfuncs text_taskfuncs;

extern void text_item_entry(vhash *item);
extern void text_task_entry(vhash *task);
extern void text_task_finish(void);

#endif
