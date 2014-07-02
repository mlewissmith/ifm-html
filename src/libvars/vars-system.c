/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup system System functions

  These functions provide an interface to various system functions.
*/

/*!
  @defgroup system_name Filename functions
  @ingroup system
*/

/*!
  @defgroup system_io I/O functions
  @ingroup system
*/

/*!
  @defgroup system_misc Miscellaneous functions
  @ingroup system
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-hash.h"
#include "vars-macros.h"
#include "vars-system.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_GETDATE
#include "getdate.h"
#endif

#ifndef IGNORE_GLOB
#include "glob.h"
#endif

#define INIT_IO                                                         \
        if (pipes == NULL) {                                            \
            pipes = vh_create();                                        \
            pipedata = vh_create();                                     \
            pipelist = vl_create();                                     \
        }

#define STAT_FILE(file)                                                 \
        if (file != NULL)                                               \
            statok = (stat(file, &statbuf) == 0)

#define BUF_APPEND(b, text)                                             \
        do {                                                            \
            if (vb_length(b) > 0) vb_putc(b, ' ');                      \
            vb_puts(b, text);                                           \
        } while (0)

#ifndef GLOB_ABORTED
#define GLOB_ABORTED GLOB_ABEND
#endif

/* File stat buffer */
static struct stat statbuf;

/* Whether last stat was successful */
static int statok = 0;

/*!
  @brief   Return the directory part of a path.
  @ingroup system_name
  @param   path Pathname.
  @return  Directory part (pointer to internal buffer).
*/
char *
v_dirname(char *path)
{
    char *cp = strrchr(path, V_PATHSEP[0]);
    static vbuffer *b = NULL;

    vb_init(b);

    if (cp == path)
        vb_puts(b, V_PATHSEP);
    else if (cp == NULL)
        vb_putc(b, '.');
    else while (path != cp)
        vb_putc(b, *path++);

    return vb_get(b);
}

/*!
  @brief   Return hash of environment variables.
  @ingroup system_misc

  @return  Hash table.
  Each key is the name of an environment variable, and its value is the
  value of the variable.
*/
vhash *
v_environ(void)
{
    /*@i@*/ extern char **environ;
    char *cp, buf[200];
    vhash *env;
    int i;

    env = vh_create();

    for (i = 0; environ[i] != NULL; i++) {
        strcpy(buf, environ[i]);
        if ((cp = strchr(buf, '=')) != NULL) {
            *cp++ = '\0';
            vh_sstore(env, buf, cp);
        }
    }

    return env;
}

/*!
  @brief   Return the filename part of a path.
  @ingroup system_name
  @param   path Pathname.
  @return  Filename part (pointer to internal buffer).
*/
char *
v_filename(char *path)
{
    char *cp = strrchr(path, V_PATHSEP[0]);
    static vbuffer *b = NULL;

    vb_init(b);

    if (cp == NULL)
        cp = path;
    else
        cp++;

    while (*cp != '\0')
        vb_putc(b, *cp++);

    return vb_get(b);
}

/*!
  @brief   Return a list of files in a directory that match a globbing pattern.
  @ingroup system_name
  @param   dir Directory.
  @param   pat Pattern to match.
  @return  List of matching files.
  @retval  NULL on failure (see below).

  Return names of files that match a globbing pattern. If \c dir is not \c
  NULL, returns a list of those files in directory \c dir which match the
  file globbing pattern \c pat. Elements of the returned list do not have
  \c dir prepended, and \c NULL is returned if \c dir can't be read.

  If \c dir is \c NULL, then the pattern is assumed to be a full-pathname
  pattern. Elements of the returned list are full pathnames, and \c NULL is
  returned if the initial pathname part of the pattern does not exist.

  A globbing pattern is a string in which several characters have special
  meanings. A '*' matches any string, including the null string. A '?'
  matches any single character. A '[...]' matches any one of the enclosed
  characters. A pair of characters separated by a minus sign denotes a \e
  range -- any character lexically between those two characters, inclusive,
  is matched. If the first character following the '[' is a '!' or a '^'
  then any character not enclosed is matched. A '-' may be matched by
  including it as the first or last character in the set. A ']' may be
  matched by including it as the first character in the set.
*/
vlist *
v_glob(char *dir, char *pat)
{
#ifndef IGNORE_GLOB
    char buf[PATH_MAX + 1];
    glob_t data;
    int i, flag;
    vlist *l;

    if (dir != NULL) {
        getcwd(buf, PATH_MAX + 1);
        if (chdir(dir) != 0)
            return NULL;
    }

    flag = glob(pat, GLOB_ERR, NULL, &data);

    if (dir != NULL && chdir(buf) != 0)
        return NULL;

    switch (flag) {
    case GLOB_NOSPACE:
        return NULL;
    case GLOB_ABORTED:
        l = NULL;
        break;
    case GLOB_NOMATCH:
        l = vl_create();
        break;
    default:
        l = vl_create();
        for (i = 0; i < data.gl_pathc; i++)
            vl_spush(l, data.gl_pathv[i]);
        break;
    }

    globfree(&data);

    return l;
#else
    v_unavailable("v_glob()");
    return NULL;
#endif
}

/*!
  @brief   Lock an already-opened stream.
  @ingroup system_io
  @param   fp Stream to lock.
  @param   locktype Type of lock.
  @param   wait Whether to wait for lock.
  @return  Whether successful.

  There are two types of lock, indicated by <tt>locktype</tt>: read locks
  (V_LOCK_READ) and write locks (V_LOCK_WRITE). Many processes can have a
  read lock on a stream, but if any process has one, no process can get a
  write lock on it until they have all been removed. Only one process can
  get a write lock on a stream, and if so, no process can get a write lock
  until it has been removed. A lock is removed when the stream is closed.

  The \c wait flag indicates whether the process should wait until a lock
  is possible, or return immediately. In any case, the return value
  indicates whether the lock was successful.
*/
int
v_lock(FILE *fp, enum v_locktype locktype, int wait)
{
#ifdef HAVE_SYS_FCNTL_H
    struct flock lock;
    int fd;

    if ((fd = fileno(fp)) < 0)
        return 0;

    lock.l_type = (locktype == V_LOCK_WRITE ? F_WRLCK : F_RDLCK);
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, (wait ? F_SETLKW : F_SETLK), &lock) < 0)
        return 0;

    return 1;
#else
    return 0;
#endif
}

/*!
  @brief   Parse a date string into a time value.
  @ingroup system_misc

  @param   string Date specification.
  Can be anything that looks vaguely like a time or date, e.g. '2pm' ,'next
  thursday'.

  @return  Time value.
  @retval  -1 if string doesn't look like a date.
*/
time_t
v_parse_date(char *string)
{
#ifdef HAVE_GETDATE
    return get_date(string, NULL);
#else
    v_unavailable("v_parse_date()");
#endif
}

/*!
  @brief   Get information about running processes.
  @ingroup system_misc
  @return  List of process entries.
  @retval  NULL if it failed.

  Each entry in the list is a hash containing the following entries:

    - \c USER -- user owning the process
    - \c PID -- process ID
    - \c PPID -- parent process ID
    - \c ENV -- explicit environment settings
    - \c PROG -- program being run
    - \c ARGS -- program arguments
*/
vlist *
v_procinfo(void)
{
#ifdef HAVE_POPEN
    char user[1024], *line, cmd[1024], linebuf[1024], *token;
    static vbuffer *env = NULL, *prog = NULL, *args = NULL;
    vlist *list = NULL;
    FILE *fp = NULL;
    int pid, ppid;
    vhash *data;

#ifdef HAVE_UID
    struct passwd *pw;
    int uid;
#endif

    /* Open pipe */
    if ((fp = popen(PS_CMD, "r")) == NULL)
        return NULL;

    /* Skip over header line */
    if (fgets(linebuf, 1024, fp) == NULL) {
        pclose(fp);
        return NULL;
    }

    list = vl_create();

    /* Read process info */
    while ((line = fgets(linebuf, 1024, fp)) != NULL) {
        /* Get the data */
#ifdef HAVE_UID
        sscanf(line, PS_FMT, &uid, &pid, &ppid, cmd);

        if ((pw = getpwuid(uid)) != NULL)
            strcpy(user, pw->pw_name);
        else
            strcpy(user, "nobody");
#else
        sscanf(line, PS_FMT, user, &pid, &ppid, cmd);
#endif

        /* Skip uninteresting processes */
        if (cmd[0] == '[')
            continue;

        /* Add process */
        data = vh_create();
        vl_ppush(list, data);

        vh_sstore(data, "USER", user);
        vh_istore(data, "PID",  pid);
        vh_istore(data, "PPID", ppid);

        /* Extract environment, program and arguments from command */
        vb_init(env);
        vb_init(prog);
        vb_init(args);

        token = strtok(cmd, " ");
        while (token != NULL) {
            if (vb_length(env) == 0 &&
                vb_length(prog) == 0 &&
                strchr(token, '=') != NULL)
                BUF_APPEND(env, token);
            else if (vb_length(prog) == 0)
                BUF_APPEND(prog, token);
            else
                BUF_APPEND(args, token);

            token = strtok(NULL, " ");
        }

        if (vb_length(env) > 0)
            vh_sstore(data, "ENV", vb_get(env));

        if (vb_length(prog) > 0)
            vh_sstore(data, "PROG", vb_get(prog));

        if (vb_length(args) > 0)
            vh_sstore(data, "ARGS", vb_get(args));
    }

    pclose(fp);
    return list;
#else
    v_unavailable("v_procinfo()");
    return NULL;
#endif
}

/*!
  @brief   Set/unset a signal handler function.
  @ingroup system_misc
  @param signal Signal to handle.
  @param func Function to call (or @c NULL to unset it).
*/
void
v_signal(int signal, void (*func)(int signal))
{
#ifdef HAVE_SIGACTION
    struct sigaction action, oldaction;
    int force = 0;

    switch (signal) {
    case SIGSEGV:
    case SIGFPE:
#ifdef SIGBUS
    case SIGBUS:
#endif
        force = 1;
        break;
    }

    action.sa_flags   = 0;
    action.sa_handler = (func != NULL ? func : SIG_DFL);
    sigemptyset(&action.sa_mask);

    sigaction(signal, NULL, &oldaction);
    if (force || oldaction.sa_handler != SIG_IGN)
        sigaction(signal, &action, NULL);
#else
    v_unavailable("v_signal()");
#endif
}

/*!
  @brief   Return list of \c stat(2) information on a file.
  @ingroup system_io

  @param   file Filename to stat.
  If \c NULL, stat information from the most recent \c stat(2) call is
  used.

  @return  List of stat info (pointer to internal buffer).
  @retval  NULL if the file doesn't exist.

  The list contains the following entries:
      -# device number of filesystem
      -# inode number
      -# file mode (type and permissions)
      -# number of (hard) links to the file
      -# numeric user ID of file's owner
      -# numer group ID of file's owner
      -# the device identifier (special files only)
      -# total size of file, in bytes
      -# last access time since the epoch
      -# last modify time since the epoch
      -# inode change time (NOT creation time!) since the epoch
      -# preferred blocksize for file system I/O
      -# actual number of blocks allocated
*/
vlist *
v_stat(char *file)
{
    static vlist *l = NULL;

    /* Get stat info */
    STAT_FILE(file);
    if (!statok)
        return NULL;

    /* Initialise */
    vl_init(l);

    /* Store info */
    vl_istore(l,  0, (int) statbuf.st_dev);
    vl_istore(l,  1, statbuf.st_ino);
    vl_istore(l,  2, statbuf.st_mode);
    vl_istore(l,  3, statbuf.st_nlink);
    vl_istore(l,  4, statbuf.st_uid);
    vl_istore(l,  5, statbuf.st_gid);

#ifdef HAVE_STRUCT_STAT_ST_RDEV
    vl_istore(l,  6, (int) statbuf.st_rdev);
#endif

    vl_istore(l,  7, statbuf.st_size);
    vl_istore(l,  8, statbuf.st_atime);
    vl_istore(l,  9, statbuf.st_mtime);
    vl_istore(l, 10, statbuf.st_ctime);

#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
    vl_istore(l, 11, statbuf.st_blksize);
#endif

#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
    vl_istore(l, 12, statbuf.st_blocks);
#endif

    return l;
}

/*!
  @brief   Return the suffix part of a path.
  @ingroup system_name
  @param   path Pathname.
  @return  Suffix part (pointer to internal buffer).
*/
char *
v_suffix(char *path)
{
    char *cp = strrchr(path, '.');
    static vbuffer *b = NULL;

    vb_init(b);

    if (cp != NULL)
        while (*cp != '\0')
            vb_putc(b, *cp++);

    return vb_get(b);
}

/*!
  @brief   Create a temporary file.
  @ingroup system_name
  @param   prefix Prefix to use.
  @param   buffer Buffer to put filename in.

  The file is created in /tmp, or the value of TMPDIR if that is set.

  @return  Buffer (modified).
  @retval  NULL if it failed (and buffer is unchanged).
*/
char *
v_tempfile(char *prefix, char *buffer)
{
    static char tmpbuf[200];
    char *dir;
    int fd;

    if ((dir = getenv("TMPDIR")) == NULL)
        dir = V_TEMPDIR;

    sprintf(tmpbuf, "%s%s%s.XXXXXX", dir, V_PATHSEP, prefix);

#ifdef HAVE_MKSTEMP
    if ((fd = mkstemp(tmpbuf)) >= 0) {
        strcpy(buffer, tmpbuf);
        close(fd);
    }
#else
    strcpy(buffer, mktemp(tmpbuf));
#endif    

    return buffer;
}

/*!
  @brief   Test a file for various attributes, via \c stat(2).
  @ingroup system_io

  @param   file Pathname to test.
  If \c NULL, the results of the most recent \c stat(2) are used.

  @param   test Type of test.
  @return  Yes or no.
*/
int
v_test(char *file, enum v_testtype test)
{
    int retval = 0;

    /* Stat the file */
    STAT_FILE(file);
    if (!statok)
        return 0;

    /* Do the test */
    switch (test) {
    case V_TEST_EXISTS:
        retval = 1;
        break;
    case V_TEST_DIR:
        retval = S_ISDIR(statbuf.st_mode);
        break;
    case V_TEST_CHR:
        retval = S_ISCHR(statbuf.st_mode);
        break;
    case V_TEST_BLK:
        retval = S_ISBLK(statbuf.st_mode);
        break;
    case V_TEST_REG:
        retval = S_ISREG(statbuf.st_mode);
        break;
    case V_TEST_FIFO:
        retval = S_ISFIFO(statbuf.st_mode);
        break;
    case V_TEST_LNK:
        retval = S_ISLNK(statbuf.st_mode);
        break;
    case V_TEST_SOCK:
        retval = S_ISSOCK(statbuf.st_mode);
        break;
    default:
        v_exception("v_test(): invalid test type");
        break;
    }

    return retval;
}
