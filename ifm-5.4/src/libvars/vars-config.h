/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Vars configuration stuff */

#ifndef VARS_CONFIG_H
#define VARS_CONFIG_H

/* Include Windoze stuff if required */
#ifdef _WINDOWS
#include "vars-win32.h"
#endif

/* Include other headers if available */
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* Stuff for getting process list */
#define PS_PROG "/bin/ps"

#if defined(_AIX) || defined(___AIX)	/* AIX 3.x */
#  define PS_CMD PS_PROG " -ekf"
#  define PS_FMT "%s %d %d %*20c %*s %[^\n]"
#elif defined(__linux)			/* Linux 0.9x */
#  define HAVE_UID
#  define PS_CMD PS_PROG " laxww"
#  define PS_FMT "%*d %d %d %d %*40c %*s %[^\n]"
#elif defined(_BSD)			/* Untested */
#  define HAVE_UID
#  define PS_CMD PS_PROG " laxww"
#  define PS_FMT "%*d %*c %d %d %d %*d %*d %*d %*x %*d %d %*15c %*s %[^\n]"
#elif defined(__convex)			/* Convex */
#  define HAVE_UID
#  define PS_CMD PS_PROG " laxww"
#  define PS_FMT "%*s %d %d %d %*d %*g %*d %*d %*21c %*s %[^\n]"
#else					/* HP-UX, A/UX etc. */
#  define PS_CMD PS_PROG " -ef"
#  define PS_FMT "%s %d %d %*20c %*s %[^\n]"
#endif

#ifdef HAVE_UID
#include <pwd.h>
#endif

/* Shut splint up about unrecognized functions */
#ifdef S_SPLINT_S
#define	M_PI 3.1415926535897932384626433832795
#define	M_E  2.7182818284590452353602874713527

extern FILE *popen(const char *command, const char *type);
extern char *strdup(const char *s);
extern char *tempnam(const char *dir, const char *pfx);
extern double drem(double x, double y);
extern double hypot(double x, double y);
extern int mkstemp(char *template);
extern int pclose(FILE *stream);
extern int strcasecmp(const char *s1, const char *s2);
#endif

extern double v_drem(double x, double y);

#endif
