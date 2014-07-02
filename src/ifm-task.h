/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Task function header */

#ifndef IFM_TASK_H
#define IFM_TASK_H

#define TASK_VERBOSE var_int("solver_messages")

/* Task types */
enum {
    T_MOVE, T_GET, T_DROP, T_GOTO, T_USER
};

/* Task list */
extern vlist *tasklist;

/* Advertised functions */
extern void check_cycles(void);
extern vhash *require_task(vhash *step);
extern void setup_tasks(void);
extern void solve_game(void);
extern void solver_msg(int level, char *fmt, ...);
extern vgraph *task_graph(void);

#endif
