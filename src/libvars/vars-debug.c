/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup debug Debugging functions

  There are several debugging features in the Vars library, which you can
  make use of via these functions.

  @par Features

  The features are controlled by debugging flags, which are:

  - @c V_DBG_MEMORY -- Keep track of the total memory currently allocated,
    and the maximum memory allocated at any one time.  At program end (if
    the feature is still enabled) the maximum memory is printed to @c
    stderr.  Uses memory tracking.

  - @c V_DBG_PROFILE -- Turn on profiling.  This starts up tracking of
    elapsed CPU time at different points in your program.

  - @c V_DBG_PTRTYPE -- Check the pointer type of arguments passed to
    library functions, and complain fatally if they're not of the required
    type.  For example, this can catch list pointers being passed to hash
    functions.

  - @c V_DBG_PTRALLOC -- Check that pointers in arguments passed to library
    functions have been allocated properly, and complain fatally if they
    haven't.  This can catch uses of pointers whose contents have since
    been deallocated.  It can't catch \e all uses, since the pointer value
    may have been reallocated to another object in the meantime.  Uses
    memory tracking.  You can implement your own pointer checking, if you
    allocate memory using the Vars allocation functions, by calling
    v_allocated().  This returns whether the pointer has been allocated, or
    1 if memory tracking isn't being done.

  - @c V_DBG_PTRS -- An abbreviation for the two previous flags together.

  - @c V_DBG_INTERN -- A special flag for debugging Vars internals.  If
    you're writing an extension, you can add messages that are printed if
    this flag is set.

  - @c V_DBG_USER -- This flag is not used by Vars, but is intended for
    user programs.  You can put debugging statements in your code and
    enable them in the same ways as standard Vars flags.

  - @c V_DBG_ALL -- A special flag you can use to turn all features on or
    off.
  .

  @par Profiling

  When profiling is enabled, a track is kept of elapsed CPU time at
  different points in your program (called checkpoints).  Then, at program
  exit (provided profiling is still enabled) a summary of elapsed real,
  user and system times (in the manner of @c time(1)) is printed at each
  checkpoint.

  One checkpoint is added by default -- at the end of the program.  You can
  add more by using v_profile().  If you want to keep your own profiling
  statistics, you can use v_times().

  @par Memory tracking

  Several of debugging features operate by using memory tracking.  This
  tells the memory allocation functions v_malloc(), v_realloc() and
  v_free() to keep track of how much memory they've handed out (and slows
  them down in the process).  Memory tracking is only done while at least
  one debugging feature that uses it is turned on.

  If memory is allocated before memory tracking is enabled, or memory
  tracking is turned off before the program finishes, the debugging
  features that use it will become confused.  So it's a fatal error to do
  these things.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-debug.h"
#include "vars-hash.h"
#include "vars-list.h"
#include "vars-macros.h"
#include "vars-memory.h"

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif

#ifndef CLK_TCK
#define CLK_TCK 1
#endif

#define MEM_TRACK_FLAGS (V_DBG_MEMORY | V_DBG_PTRALLOC)

#define V_DEBUG_OFF                                                     \
        if (v_debug_flags) {                                            \
                debug_save = v_debug_flags;                             \
                v_debug_flags = 0;                                      \
        }

#define V_DEBUG_ON                                                      \
        v_debug_flags = debug_save

#define CHECK_ENV(var)                                                  \
        if (getenv(#var) != NULL) v_debug(var)

/* Debugging flags */
int v_debug_flags = 0;

/* Whether tracking memory usage */
int v_memory_track = 0;

/* Whether memory has been allocated yet */
int v_memory_allocated = 0;

/* Saved debugging flags */
static int debug_save = 0;

/* Hash of allocated memory chunks */
static vhash *memory = NULL;

/* Total memory allocated */
static size_t total_memory = 0;

/* Maximum memory allocated */
static size_t max_memory = 0;

/* Timing profile entries */
static vlist *time_profile = NULL;

/* Internal functions */
static void exit_function(void);
static void print_timings(void);

/*!
  @brief   Return whether a pointer is allocated.
  @ingroup debug
  @param   ptr Pointer.
  @return  Yes or no.
*/
int
v_allocated(void *ptr)
{
    char buf[V_HEXSTRING_SIZE];
    int result;

    V_DEBUG_OFF;

    if (!v_memory_track)
        result = 1;
    else if (memory == NULL)
        result = 0;
    else
        result = vh_exists(memory, vh_pkey_buf(ptr, buf));

    V_DEBUG_ON;

    return result;
}

/*!
  @brief   Turn on debugging flag.
  @ingroup debug
  @param   flag Debugging flag.
*/
void
v_debug(int flag)
{
    static int addexit = 0;

    /* Set flags */
    v_debug_flags |= flag;

    /* Update memory track flag */
    flag = (v_debug_flags & MEM_TRACK_FLAGS);

    if (!v_memory_track && flag && v_memory_allocated)
        v_fatal("v_debug(): memory tracking must be active before allocation");

    v_memory_track = flag;

    /* Initialise times if required */
    if (v_debug_flags & V_DBG_PROFILE)
        v_times(NULL, NULL, NULL);

    /* Add exit function if required */
    if (!addexit) {
        atexit(exit_function);
        addexit = 1;
    }
}

/*!
  @brief   Set debug flags according to environment variables.
  @ingroup debug

  Turn on debugging features according to environment variables.  For each
  debugging feature, if an environment variable having the same name is
  set, then that feature is turned on.  If a program uses this function, it
  should be called before any other Vars functions.
*/
void
v_debug_env(void)
{
    CHECK_ENV(V_DBG_ALL);
    CHECK_ENV(V_DBG_INTERN);
    CHECK_ENV(V_DBG_MEMORY);
    CHECK_ENV(V_DBG_PROFILE);
    CHECK_ENV(V_DBG_PTRALLOC);
    CHECK_ENV(V_DBG_PTRS);
    CHECK_ENV(V_DBG_PTRTYPE);
    CHECK_ENV(V_DBG_USER);
}

/*!
  @brief   Turn off debugging flag.
  @ingroup debug
  @param   flag Debugging flag.
*/
void
v_nodebug(int flag)
{
    /* Set flags */
    v_debug_flags &= ~flag;

    /* Update memory track flag */
    flag = (v_debug_flags & MEM_TRACK_FLAGS);

    if (v_memory_track && !flag)
        v_fatal("v_nodebug(): memory tracking must be active "
                "until program end");

    v_memory_track = flag;
}

/*!
  @brief   Add a timing profile entry.
  @ingroup debug
  @param   fmt Format string.

  Add a profile checkpoint.  If profiling is enabled, then the elapsed
  real, user and system times between this checkpoint and the last (or
  program start, if this is the first) are stored.  The specified message
  is printed out with these statistics at program end.
*/
void
v_profile(char *fmt, ...)
{
    double real, user, sys;
    vlist *entry;
    V_BUF_DECL;
    char *msg;

    if (!V_DEBUG(V_DBG_PROFILE))
        return;

    /* Get times */
    if (!v_times(&real, &user, &sys))
        return;

    V_DEBUG_OFF;

    /* Add new entry to list */
    V_BUF_FMT(fmt, msg);

    entry = vl_create_size(4);
    vl_sstore(entry, 0, msg);
    vl_dstore(entry, 1, real);
    vl_dstore(entry, 2, user);
    vl_dstore(entry, 3, sys);

    if (time_profile == NULL)
        time_profile = vl_create();

    vl_ppush(time_profile, entry);

    V_DEBUG_ON;
}

/*!
  @brief   Get real, user and system times.
  @ingroup debug
  @param[out]   real Pointer to real time or NULL.
  @param[out]   user Pointer to user time or NULL.
  @param[out]   sys Pointer to system time or NULL.
  @return  Whether successful.

  Return the elapsed real, user and system times since the last time this
  function was called (or program start, if this is the first).
*/
int
v_times(double *real, double *user, double *sys)
{
#ifdef HAVE_SYS_TIMES_H
    static double ticksize = -1.0;
    static int tick_base = 0;
    static int real_old = 0;
    static int user_old = 0;
    static int sys_old = 0;
    static struct tms tbuf;
    clock_t ticks;

    /* Get times */
    if ((ticks = times(&tbuf)) < 0)
        return 0;

    if (tick_base == 0)
        tick_base = ticks;

    ticks -= tick_base;

    /* Initialise ticksize */
    if (ticksize < 0)
        ticksize = (double) CLK_TCK;

    /* Calculate times in seconds */
    if (real != NULL)
        *real = (ticks - real_old) / ticksize;

    if (user != NULL)
        *user = (tbuf.tms_utime - user_old) / ticksize;

    if (sys != NULL)
        *sys = (tbuf.tms_stime - sys_old) / ticksize;

    /* Save old values */
    real_old = ticks;
    user_old = tbuf.tms_utime;
    sys_old = tbuf.tms_stime;

    return 1;
#else
    return 0;
#endif
}

/* Record change in allocated memory */
void
v_memchange(int type, void *ptr, void *oldptr, size_t size)
{
    char buf[V_HEXSTRING_SIZE];
    size_t diff;

    V_DEBUG_OFF;

    /* Initialise */
    if (memory == NULL)
        memory = vh_create_size(1000);

    /* Update pointer memory values */
    switch (type) {

    case V_MEM_MALLOC:
        diff = size;
        vh_istore(memory, vh_pkey_buf(ptr, buf), size);
        break;

    case V_MEM_REALLOC:
        diff = size - vh_iget(memory, vh_pkey_buf(oldptr, buf));
        vh_delete(memory, vh_pkey_buf(oldptr, buf));
        vh_istore(memory, vh_pkey_buf(ptr, buf), size);
        break;

    case V_MEM_FREE:
        diff = (size_t) -vh_iget(memory, vh_pkey_buf(ptr, buf));
        vh_delete(memory, vh_pkey_buf(ptr, buf));
        break;
    }

    /* Update totals */
    total_memory += diff;
    if (max_memory < total_memory)
        max_memory = total_memory;

    V_DEBUG_ON;
}

/* General exit function */
static void
exit_function(void)
{
    /* Print timing profile if required */
    if (V_DEBUG(V_DBG_PROFILE)) {
        v_profile("End of program");
        print_timings();
    }

    /* Print memory usage if required */
    if (V_DEBUG(V_DBG_MEMORY))
        fprintf(stderr, "Vars used %d bytes\n", max_memory);
}

/* Print timing profile */
static void
print_timings(void)
{
    double real, user, sys, rtotal = 0.0, utotal = 0.0, stotal = 0.0;
    static char *fmt = "%-50s   %6s   %6s   %6s\n";
    char *str, rs[10], us[10], ss[10];
    vlist *list, *entry;
    int textwidth = 50;
    viter i, j;

    if (time_profile == NULL || vl_length(time_profile) == 0) {
        fprintf(stderr, "No profile points encountered\n");
        return;
    }

    fprintf(stderr, fmt, "Event", "Real", "User", "Sys");
    fprintf(stderr, "\n");

    v_iterate(time_profile, i) {
        entry = vl_iter_pval(i);

        str = vl_sgetref(entry, 0);
        real = vl_dget(entry, 1);
        user = vl_dget(entry, 2);
        sys = vl_dget(entry, 3);

        rtotal += real;
        utotal += user;
        stotal += sys;

        list = vl_filltext(str, textwidth);
        str = vl_sshift(list);

        sprintf(rs, "%6.2f", real);
        sprintf(us, "%6.2f", user);
        sprintf(ss, "%6.2f", sys);

        fprintf(stderr, fmt, str, rs, us, ss);
        v_iterate(list, j)
            fprintf(stderr, "%s\n", vl_iter_svalref(j));
    }

    sprintf(rs, "%6.2f", rtotal);
    sprintf(us, "%6.2f", utotal);
    sprintf(ss, "%6.2f", stotal);

    fprintf(stderr, "\n");
    fprintf(stderr, fmt, "Total", rs, us, ss);
}
