/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/* Windoze-specific config stuff */

#ifndef VARS_WIN32_H
#define VARS_WIN32_H

/* Path separator */
#define V_PATHSEP "\\"

/* Temporary directory */
#define V_TEMPDIR "."

/* Indicate what's available */
#define HAVE_ALLOCA 1
#define HAVE_BCOPY 1
#define HAVE_BZERO 1
#define HAVE_ISASCII 1
#define HAVE_MEMCPY 1
#define HAVE_MEMORY_H 1
#define HAVE_POPEN 1
#define HAVE_PROCESS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRUCT_STAT_ST_RDEV 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1

#define IGNORE_GLOB 1

#define STDC_HEADERS 1

/* Disable debugging stuff */
#undef V_DEBUG
#define V_DEBUG(flag) 0

/* Define missing stat() stuff */
#define	ISTYPE(mode, mask) (((mode) & _S_IFMT) == (mask))

#define	S_ISDIR(mode)   ISTYPE((mode), _S_IFDIR)
#define	S_ISCHR(mode)	ISTYPE((mode), _S_IFCHR)
#define	S_ISREG(mode)	ISTYPE((mode), _S_IFREG)
#define S_ISFIFO(mode)	ISTYPE((mode), _S_IFIFO)
#define	S_ISBLK(mode)	0
#define S_ISLNK(mode)   0
#define S_ISSOCK(mode)  0

/* Define aliases for various functions */
#define popen      _popen
#define pclose     _pclose
#define alloca     _alloca
#define vsnprintf  _vsnprintf
#define strcasecmp strcmp

/* Define missing constants */
#define	M_PI 3.1415926535897932384626433832795
#define	M_E  2.7182818284590452353602874713527

#endif
