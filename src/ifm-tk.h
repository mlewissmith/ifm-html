/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Tcl/Tk output driver header */

#ifndef IFM_TK_H
#define IFM_TK_H

extern mapfuncs tk_mapfuncs;
extern itemfuncs tk_itemfuncs;
extern taskfuncs tk_taskfuncs;
extern errfuncs tk_errfuncs;

extern void tk_map_start(void);
extern void tk_map_section(vhash *sect);
extern void tk_map_room(vhash *room);
extern void tk_map_link(vhash *link);
extern void tk_item_start(void);
extern void tk_item_entry(vhash *item);
extern void tk_item_finish(void);
extern void tk_task_start(void);
extern void tk_task_entry(vhash *task);
extern void tk_task_finish(void);
extern void tk_warning(char *file, int line, char *msg);
extern void tk_error(char *file, int line, char *msg);

#endif
