/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup options Option parsing

  These functions are available as shortcuts to deal with program option
  parsing. They gather up all the option information into a single hash, so
  you don't have to declare a whole bunch of variables to collect it.
*/

/*!
  @defgroup options_declare Declaration functions
  @ingroup options

  Two types of option parsing are supported: single-letter options and long
  options. Single-letter options have the advantage that you can bunch
  several together in a single 'option', but the disadvantage that they're
  less readable and limited in availability. Long options may not be
  bundled together, but may be abbreviated as long as they are kept unique.

  In both cases, the procedure for use is the same. First you declare all
  the options you want to recognize, and then you parse them. There are two
  ways to declare options:

  - Use the v_option() function, passing in a flag indicating which type of
    option it is.  You can then extract the option settings from the hash
    returned from v_getopts().
  - Use the longer-named functions v_option_flag(), v_option_arg(), etc.,
    which can also accept a pointer to a variable which is set if the
    option is activated.  Note that the variable is not changed if the
    option isn't selected, so you should give it a default value.

  If there are many options to your program, it may be worthwhile ordering
  them into related groups. You can divide up groups of options in the
  usage message by using calls to v_optgroup().
*/

/*!
  @defgroup options_parse Parsing functions
  @ingroup options

  Once you've declared all your options, you can parse the command line
  using the v_getopts() function.

  Many programs have a help option which displays a short usage message,
  describing the recognized options. After all your options have been
  declared, you can use the v_usage() function to do that.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-getopt.h"
#include "vars-macros.h"
#include "vars-memory.h"

/*@-noparams@*/
#include "getopt.h"
/*@=noparams@*/

#define SET_VAR(type, var, val)                                         \
        if (var != NULL) *((type *) var) = val

#define OPTION(letter, name, otype, argname, vtype, var, fmt) {         \
        char *desc = NULL;                                              \
        V_BUF_DECL;                                                     \
        if (fmt != NULL) V_BUF_FMT(fmt, desc);                          \
        option(letter, name, otype, argname, vtype, var, desc);         \
}

/* Option lists */
static vlist *option_list = NULL;

static vlist *short_optlist = NULL;
static vlist *long_optlist = NULL;

/* Name -> option mapping */
static vhash *short_opthash = NULL;
static vhash *long_opthash = NULL;

/* Current option group description */
static char *optgroup = NULL;

/* Internal functions */
static void option(char optletter, char *optname, enum v_oflag type,
                   char *argname, int vtype, void *var, char *fmt, ...);
static char *optstring(void);
static int set_long_option(vhash *opts, char *opt, char *arg);
static int set_option(vhash *opts, char *name, enum v_oflag type, void *var,
                      enum v_stype vtype, char *arg);
static int set_short_option(vhash *opts, char opt, char *arg);

/*!
  @brief   Parse command-line options.
  @ingroup options_parse
  @param   argc From main().
  @param   argv From main().
  @return  Hash of parsed options.
  @retval  NULL if there were unrecognized or invalid option(s).

  The returned hash contains an entry per option found, so you can use
  vh_exists() to see if an option has been set.

  The hash value of each entry depends on the type of option: V_OPT_ARG
  gives the argument itself, V_OPT_OPTARG gives the argument if specified
  (or the empty string if not), V_OPT_FLAG gives 1, and V_OPT_LIST gives a
  pointer to the list of arguments specified. If the option takes an
  argument, the hash value is the value of the argument. If not, the hash
  value is 1. If the option is of type V_OPT_LIST, the hash value is a
  pointer to a list of arguments.  If the option is V_OPT_RANGE, the hash
  value is a pointer to a list of flags.

  Two other special entries are set: \c PROG is the name of the program,
  and \c ARGS is a list of the remaining command-line values following the
  options.

  If no long options have been set, the options are parsed as single-letter
  options. Otherwise, the function tries by default to find a long option
  match for arguments. If that fails, it attempts a single-letter option
  match. Users can force a long-option-only match by using \c -- as the
  option prefix string, rather than just \c - .

  Note that for options of type V_OPT_OPTARG, if something follows the
  option then that is taken to be its argument, even if you intended it to
  be another option.

  Note that if a long option has an equivalent single-letter option, then
  both entries in the returned hash are set (and are equal) if either
  method is used to specify it, so either can be used to check the option.

  By default, this function permutes the command-line arguments so that
  eventually all the non-options are at the end. This allows options to be
  given in any order. Users can suppress this behaviour by setting the
  environment variable \c POSIXLY_CORRECT . The special argument \c --
  forces end of option-scanning regardless of permutation.
*/
vhash *
v_getopts(int argc, char *argv[])
{
    int nopts, i, c, type, option_index;
    struct option *optlist;
    vhash *opt, *opts;
    vlist *args;
    char *name;

    /* Initialise */
    opts = vh_create();
    args = vl_create();
    vh_sstore(opts, "PROG", argv[0]);
    vh_pstore(opts, "ARGS", args);

    /* Parse arguments */
    if (long_optlist == NULL) {
        /* Single-letter options only */
        while ((c = getopt(argc, argv, optstring())) != EOF) {
            if (c == '?')
                return NULL;
            else if (!set_short_option(opts, c, optarg))
                return NULL;
        }
    } else {
        /* Long options */
        nopts = (long_optlist == NULL ? 0 : vl_length(long_optlist));
        optlist = V_ALLOCA(struct option, nopts + 1);

        /* Build internal options */
        for (i = 0; i < nopts; i++) {
            opt = vl_pget(long_optlist, i);
            type = vh_iget(opt, "TYPE");

            optlist[i].name = vh_sgetref(opt, "LONG");
            optlist[i].flag = NULL;
            optlist[i].val = 0;

            switch (type) {

            case V_OPT_ARG:
            case V_OPT_LIST:
            case V_OPT_RANGE:
                optlist[i].has_arg = required_argument;
                break;

            case V_OPT_FLAG:
                optlist[i].has_arg = no_argument;
                break;

            case V_OPT_OPTARG:
                optlist[i].has_arg = optional_argument;
                break;
            }
        }

        /* Add terminator */
        optlist[nopts].name = NULL;

        /* Parse options */
        while ((c = getopt_long_only(argc, argv, optstring(),
                                     optlist, &option_index)) != EOF) {
            switch (c) {
            case '?':
                return NULL;
            case 0:
                /* Long option */
                name = (char *) optlist[option_index].name;
                if (!set_long_option(opts, name, optarg))
                    return NULL;
                break;
            default:
                /* Short option */
                if (!set_short_option(opts, c, optarg))
                    return NULL;
                break;
            }
        }

    }

    /* Collect remaining arguments */
    while (optind < argc)
        vl_spush(args, argv[optind++]);

    return opts;
}

/*!
  @brief   Get remaining arguments after options were parsed.
  @ingroup options_parse
  @param   opts Result of calling v_getopts().
  @return  List of arguments.
  @retval  NULL if there were unrecognized or invalid option(s).
*/
vlist *
v_getargs(vhash *opts)
{
    if (opts != NULL)
        return vh_pget(opts, "ARGS");

    return NULL;
}

/*!
  @brief   Declare an option group.
  @ingroup options_declare
  @param   desc Group description.

  Attach an option grouping description to the next option declared by
  v_option(). Then, just before that option is printed by v_usage(), this
  descriptive text is printed, between two newlines.
*/
void
v_optgroup(char *desc)
{
    optgroup = desc;
}

/*!
  @brief   Declare a command-line option.
  @ingroup options_declare
  @param   optletter Single-character option (or \c '\\0' ).
  @param   optname Long option (or \c NULL ).

  @param   type Option type.
  This specifies what sort of option it is: V_OPT_ARG (it requires an
  argument), V_OPT_OPTARG (it takes an optional argument), V_OPT_FLAG (it
  takes no argument), V_OPT_LIST (it requires an argument, and multiple
  invocations of the option will concatenate them into a list), or
  V_OPT_RANGE (it requires an argument which is a list of numbers or
  ranges, separated by commas -- e.g. 1,2,5-7,10 -- and returns a list of
  flags representing the numbers selected).

  @param   argname Name of the option argument (if any).
  This will be used in v_usage(). If it is \c NULL, then the string "arg"
  is printed instead. If \c type is V_OPT_FLAG, then \c argname is ignored.

  @param   fmt Format string for description (or \c NULL ).
  This will also be used in the usage message. This argument is treated as
  a printf()-like format string, and all other arguments following this one
  are printed using it. If it is \c NULL, then no entry will be printed by
  v_usage() for this option.

  If \c optletter is not \c '\\0', that is the single letter which invokes
  it. If \c optname is not \c NULL, that is the long option name which
  invokes it. Obviously, at least one of these must be set.
*/
void
v_option(char optletter, char *optname, enum v_oflag type, char *argname,
         char *fmt, ...)
{
    OPTION(optletter, optname, type, argname, V_TYPE_NULL, NULL, fmt);
}

/*!
  @brief   Declare a command-line option that takes an argument.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   argname See v_option().
  @param   type Type of associated variable.
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_arg(char optletter, char *optname, char *argname,
             enum v_stype type, void *var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_ARG, argname, type, var, fmt);
}

/*!
  @brief   Declare a command-line option that takes a double argument.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   argname See v_option().
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_double(char optletter, char *optname, char *argname,
                double *var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_ARG, argname, V_TYPE_DOUBLE, var, fmt);
}

/*!
  @brief   Declare a command-line option that is a flag.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_flag(char optletter, char *optname, int *var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_FLAG, NULL, V_TYPE_INT, var, fmt);

    if (var != NULL)
        *var = 0;
    else
        v_fatal("v_option_flag(): no flag variable defined");
}

/*!
  @brief   Declare a command-line option that takes a float argument.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   argname See v_option().
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_float(char optletter, char *optname, char *argname,
               float *var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_ARG, argname, V_TYPE_FLOAT, var, fmt);
}

/*!
  @brief   Declare a command-line option that takes an integer argument.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   argname See v_option().
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_int(char optletter, char *optname, char *argname,
             int *var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_ARG, argname, V_TYPE_INT, var, fmt);
}

/*!
  @brief   Declare a command-line option that is a list.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   argname See v_option().
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_list(char optletter, char *optname, char *argname,
              vlist **var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_LIST, argname, V_TYPE_POINTER, var, fmt);
}

/*!
  @brief   Declare a command-line option that takes an optional argument.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   argname See v_option().
  @param   type Type of associated variable.
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_optarg(char optletter, char *optname, char *argname,
                enum v_stype type, void *var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_OPTARG, argname, V_TYPE_POINTER, var, fmt);
}

/*!
  @brief   Declare a command-line option that is a range of values.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   argname See v_option().
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_range(char optletter, char *optname, char *argname,
               vlist **var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_RANGE, argname, V_TYPE_POINTER, var, fmt);
}

/*!
  @brief   Declare a command-line option that takes a string argument.
  @ingroup options_declare
  @param   optletter See v_option().
  @param   optname See v_option().
  @param   argname See v_option().
  @param   var Pointer to associated variable.
  @param   fmt See v_option().
*/
void
v_option_string(char optletter, char *optname, char *argname,
                char **var, char *fmt, ...)
{
    OPTION(optletter, optname, V_OPT_ARG, argname, V_TYPE_STRING, var, fmt);
}

/*!
  @brief   Print a usage message and exit.
  @ingroup options_parse
  @param   fmt Format string.

  Print a usage message to stdout. The message consists of a single line
  formed via \c printf(3) using the given format and arguments, followed by
  lines describing each option in the order the options were declared. An
  option description consists of the option name itself preceded by '-',
  followed an argument string (if it takes one) and its description. The
  argument string is the word 'arg' surrounded by brackets: <> if
  mandatory, [] if optional, () if a list and {} if a range.
*/
void
v_usage(char *fmt, ...)
{
    char *desc, *str, *sname, *lname, *argname, *b1, *b2;
    int len = 0, type, i, linewidth = 75, arg;
    vlist *entries, *list;
    vhash *entry, *opt;
    viter iter;
    V_BUF_DECL;

    /* Build usage message entries (if any) */
    entries = vl_create();

    if (option_list != NULL) {
        v_iterate(option_list, iter) {
            opt = vl_iter_pval(iter);

            sname = vh_sgetref(opt, "SHORT");
            lname = vh_sgetref(opt, "LONG");
            desc = vh_sgetref(opt, "DESC");
            type = vh_iget(opt, "TYPE");
            argname = vh_sgetref(opt, "ARGNAME");

            if (desc == NULL)
                continue;

            if (strlen(sname) == 0) {
                if (short_optlist == NULL)
                    V_BUF_SET1("-%s", lname);    
                else
                    V_BUF_SET1("    -%s", lname);
            } else if (strlen(lname) == 0) {
                V_BUF_SET1("-%s", sname);
            } else {
                V_BUF_SET2("-%s, -%s", sname, lname);
            }

            arg = 1;
            switch (type) {
            case V_OPT_ARG:
                b1 = " <", b2 = ">";
                break;
            case V_OPT_OPTARG:
                b1 = "[=", b2 = "]";
                break;
            case V_OPT_LIST:
                b1 = " (", b2 = ")";
                break;
            case V_OPT_RANGE:
                b1 = " {", b2 = "}";
                break;
            default:
                arg = 0;
                break;
            }

            entry = vh_create();
            vl_ppush(entries, entry);
            vh_sstore(entry, "OPTS", V_BUF_VAL);

            if (arg) {
                V_BUF_SET3("%s%s%s",
                           b1, (argname != NULL ? argname : "arg"), b2);
                vh_sstore(entry, "ARG", V_BUF_VAL);
            }

            vh_sstore(entry, "DESC", desc);
            if (vh_exists(opt, "GROUP"))
                vh_sstore(entry, "GROUP", vh_sgetref(opt, "GROUP"));
        }
    }

    /* Make option strings */
    v_iterate(entries, iter) {
        entry = vl_iter_pval(iter);
        str = vh_sgetref(entry, "OPTS");

        if (vh_exists(entry, "ARG"))
            str = V_BUF_SET2("%s%s", str, vh_sgetref(entry, "ARG"));

        vh_sstore(entry, "NAME", str);
        if (strlen(str) > (size_t) len)
            len = strlen(str);
    }

    /* Print header line */
    V_BUF_FMT(fmt, str);
    puts(str);

    /* Build format string */
    V_BUF_SET1("   %%-%ds %%s\n", len + 1);

    /* Print options */
    v_iterate(entries, iter) {
        entry = vl_iter_pval(iter);

        if (vh_exists(entry, "GROUP"))
            printf("\n%s\n", vh_sgetref(entry, "GROUP"));

        list = vl_filltext(vh_sgetref(entry, "DESC"), linewidth - len - 6);
        for (i = 0; i < vl_length(list); i++)
            printf(V_BUF_VAL, (i > 0 ? "" : vh_sgetref(entry, "NAME")),
                   vl_sgetref(list, i));
    }
}

/*!
  @brief   Parse a list argument into a list of flags.
  @ingroup options_parse
  @param   list
  @return  List of flags.
  @retval  NULL if the specification is invalid.

  This is used internally by v_getopts(), but is generally useful.
*/
vlist *
vl_parse_list(char *list)
{
    int flag, i, min, max;
    vlist *parts, *flags;
    viter iter;
    char *part;

    flags = vl_create();
    parts = vl_split(list, ",");

    v_iterate(parts, iter) {
        part = vl_iter_svalref(iter);
        if (strchr(part, '-') != NULL) {
            if (sscanf(part, "%d-%d", &min, &max) < 2)
                return NULL;
            if (min < 0 || max < 0 || min > max)
                return NULL;
            for (i = min; i <= max; i++)
                vl_istore(flags, i, 1);
        } else {
            if (sscanf(part, "%d", &flag) < 1)
                return NULL;
            vl_istore(flags, flag, 1);
        }
    }

    vl_destroy(parts);
    return flags;
}

/* Internal option declarator */
static void
option(char optletter, char *optname, enum v_oflag type, char *argname,
       int vtype, void *var, char *fmt, ...)
{
    vhash *opt = vh_create();
    char *desc = NULL;
    V_BUF_DECL;

    /* Initialise */
    if (option_list == NULL)
        option_list = vl_create();

    if (optletter == '\0' && optname == NULL)
        v_fatal("v_option(): no option letter or long name specified");

    switch (type) {
    case V_OPT_ARG:
    case V_OPT_OPTARG:
    case V_OPT_FLAG:
    case V_OPT_LIST:
    case V_OPT_RANGE:
        break;
    default:
        v_fatal("v_option(): invalid option flag");
    }

    vl_ppush(option_list, opt);

    if (fmt != NULL)
        V_BUF_FMT(fmt, desc);

    vh_istore(opt, "TYPE", type);
    vh_sstore(opt, "ARGNAME", argname);
    vh_sstore(opt, "DESC", desc);
    vh_istore(opt, "VTYPE", vtype);
    vh_pstore(opt, "VAR", var);

    if (optgroup != NULL) {
        vh_sstore(opt, "GROUP", optgroup);
        optgroup = NULL;
    }

    /* Add single-letter option if required */
    if (optletter != '\0') {
        if (short_optlist == NULL) {
            short_optlist = vl_create();
            short_opthash = vh_create();
        }

        V_BUF_SET1("%c", optletter);
        vh_sstore(opt, "SHORT", V_BUF_VAL);

        if (vh_exists(short_opthash, V_BUF_VAL))
            v_fatal("v_option(): option '%c' declared twice", optletter);

        vl_ppush(short_optlist, opt);
        vh_pstore(short_opthash, V_BUF_VAL, opt);
    }

    /* Add long option if required */
    if (optname != NULL) {
        if (strlen(optname) < (size_t) 2)
            v_fatal("v_option(): long name must be at least 2 characters");

        if (long_optlist == NULL) {
            long_optlist = vl_create();
            long_opthash = vh_create();
        }

        vh_sstore(opt, "LONG", optname);

        if (vh_exists(long_opthash, V_BUF_VAL))
            v_fatal("v_option(): option '%s' declared twice", optname);

        vl_ppush(long_optlist, opt);
        vh_pstore(long_opthash, optname, opt);
    }
}

/* Return short option string */
static char *
optstring(void)
{
    viter iter;
    vhash *opt;
    char *name;
    V_BUF_DECL;
    int type;

    if (short_optlist == NULL)
        return "";

    V_BUF_INIT;

    v_iterate(short_optlist, iter) {
        opt = vl_iter_pval(iter);
        name = vh_sgetref(opt, "SHORT");
        type = vh_iget(opt, "TYPE");

        V_BUF_ADD(name);

        switch (type) {

        case V_OPT_ARG:
        case V_OPT_LIST:
        case V_OPT_RANGE:
            V_BUF_ADD(":");
            break;

        case V_OPT_FLAG:
            break;

        case V_OPT_OPTARG:
            V_BUF_ADD("::");
            break;
        }
    }

    return V_BUF_VAL;
}

/* Set a long option */
static int
set_long_option(vhash *opts, char *opt, char *arg)
{
    int type, vtype;
    vhash *option;
    char *name;
    void *var;

    /* Set the option */
    option = vh_pget(long_opthash, opt);
    name = vh_sgetref(option, "LONG");
    type = vh_iget(option, "TYPE");
    var = vh_pget(option, "VAR");
    vtype = vh_iget(option, "VTYPE");

    if (!set_option(opts, name, type, var, vtype, arg))
        return 0;

    /* Set equivalent short option as well */
    name = vh_sgetref(option, "SHORT");
    if (strlen(name) > 0 && !set_option(opts, name, type, NULL, vtype, arg))
        return 0;

    return 1;
}

/* Set an option */
static int
set_option(vhash *opts, char *name, enum v_oflag type,
           void *var, enum v_stype vtype, char *arg)
{
    char *progname = vh_sgetref(opts, "PROG");
    vlist *list, *flags;
    int i, len, ival;
    double dval;
    float fval;

    switch (type) {

    case V_OPT_ARG:
    case V_OPT_OPTARG:
        vh_sstore(opts, name, (arg == NULL ? "" : arg));

        switch (vtype) {

        case V_TYPE_INT:
            if (sscanf(arg, "%d", &ival)) {
                SET_VAR(int, var, ival);
            } else {
                fprintf(stderr, "%s: expected integer: %s\n", progname, arg);
                return 0;
            }

            break;

        case V_TYPE_FLOAT:
            if (sscanf(arg, "%f", &fval)) {
                SET_VAR(float, var, fval);
            } else {
                fprintf(stderr, "%s: expected float: %s\n", progname, arg);
                return 0;
            }

            break;

        case V_TYPE_DOUBLE:
            if (sscanf(arg, "%lf", &dval)) {
                SET_VAR(double, var, dval);
            } else {
                fprintf(stderr, "%s: expected double: %s\n", progname, arg);
                return 0;
            }

            break;

        case V_TYPE_STRING:
            SET_VAR(char *, var, arg);
            break;
        }

        break;

    case V_OPT_LIST:
        if ((list = vh_pget(opts, name)) == NULL) {
            list = vl_create();
            vh_pstore(opts, name, list);
        }

        vl_spush(list, arg);
        SET_VAR(vlist *, var, list);
        break;

    case V_OPT_RANGE:
        if ((list = vh_pget(opts, name)) == NULL) {
            list = vl_create();
            vh_pstore(opts, name, list);
        }

        if ((flags = vl_parse_list(arg)) != NULL) {
            len = vl_length(flags);
            for (i = 0; i < len; i++)
                if (vl_iget(flags, i))
                    vl_istore(list, i, 1);
            vl_destroy(flags);
        } else {
            fprintf(stderr, "%s: invalid range: %s\n", progname, arg);
            return 0;
        }

        SET_VAR(vlist *, var, list);
        break;

    case V_OPT_FLAG:
        vh_istore(opts, name, 1);
        SET_VAR(int, var, 1);
        break;
    }

    return 1;
}

/* Set a short option */
static int
set_short_option(vhash *opts, char opt, char *arg)
{
    int type, vtype;
    vhash *option;
    V_BUF_DECL;
    char *name;
    void *var;

    /* Set the option */
    V_BUF_SET1("%c", opt);
    option = vh_pget(short_opthash, V_BUF_VAL);
    name = vh_sgetref(option, "SHORT");
    type = vh_iget(option, "TYPE");
    var = vh_pget(option, "VAR");
    vtype = vh_iget(option, "VTYPE");

    if (!set_option(opts, name, type, var, vtype, arg))
        return 0;

    /* Set equivalent long option as well */
    name = vh_sgetref(option, "LONG");
    if (strlen(name) > 0 && !set_option(opts, name, type, NULL, vtype, arg))
        return 0;

    return 1;
}
