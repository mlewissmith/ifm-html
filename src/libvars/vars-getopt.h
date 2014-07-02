/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @file
  @brief Option parsing functions.
*/

#ifndef VARS_GETOPT_H
#define VARS_GETOPT_H

#include <vars-hash.h>

/*! Option parsing flags */
enum v_oflag {
    V_OPT_ARG,                  /*!< Requires an argument */
    V_OPT_OPTARG,               /*!< Takes an optional argument */
    V_OPT_FLAG,                 /*!< Takes no argument */
    V_OPT_LIST,                 /*!< Is a listable argument */
    V_OPT_RANGE                 /*!< Is a numeric range argument */
};

#ifdef __cplusplus
extern "C" {
#endif

extern vhash *v_getopts(int argc, char *argv[]);
extern vlist *v_getargs(vhash *opts);
extern void v_optgroup(char *desc);
extern void v_option(char optletter, char *optname, enum v_oflag type,
                     char *argname, char *fmt, ...);
extern void v_option_arg(char optletter, char *optname, char *argname,
                         enum v_stype type, void *var, char *fmt, ...);
extern void v_option_double(char optletter, char *optname, char *argname,
                            double *var, char *fmt, ...);
extern void v_option_flag(char optletter, char *optname, int *var, char *fmt,
                          ...);
extern void v_option_float(char optletter, char *optname, char *argname,
                           float *var, char *fmt, ...);
extern void v_option_int(char optletter, char *optname, char *argname,
                         int *var, char *fmt, ...);
extern void v_option_list(char optletter, char *optname, char *argname,
                          vlist **var, char *fmt, ...);
extern void v_option_optarg(char optletter, char *optname, char *argname,
                            enum v_stype type, void *var, char *fmt, ...);
extern void v_option_range(char optletter, char *optname, char *argname,
                           vlist **var, char *fmt, ...);
extern void v_option_string(char optletter, char *optname, char *argname,
                            char **var, char *fmt, ...);
extern void v_usage(char *fmt, ...);
extern vlist *vl_parse_list(char *list);

#ifdef __cplusplus
}
#endif

#endif
