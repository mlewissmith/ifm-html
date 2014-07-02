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
  @brief Freeze/thaw functions.
*/

#ifndef VARS_FREEZE_H
#define VARS_FREEZE_H

/*! @brief Thaw token types. */
enum v_tokentype {
    V_TOKEN_INT = 256,          /*!< Integer */
    V_TOKEN_REAL,               /*!< Real number */
    V_TOKEN_VTYPE,              /*!< Vars object type */
    V_TOKEN_STRING,             /*!< String */
    V_TOKEN_REF,                /*!< Reference */
    V_TOKEN_NULL,               /*!< NULL */
    V_TOKEN_UNDEF,              /*!< UNDEF */
    V_TOKEN_ID,                 /*!< ID name */
    V_TOKEN_ERROR               /*!< Error */
};

/* Standard token strings */
#define V_TOKEN_UNDEF_STRING   "UNDEF"
#define V_TOKEN_NULL_STRING    "NULL"
#define V_TOKEN_REF_STRING     "REF"
   
#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Integer token value. */
extern int v_thaw_ivalue;

/*! @brief Real token value. */
extern double v_thaw_dvalue;

/*! @brief String token value. */
extern char *v_thaw_svalue;

/*! @brief Pointer token value. */
extern void *v_thaw_pvalue;

extern int v_freeze(void *ptr, FILE *fp);
extern int v_freeze_file(void *ptr, char *file);
extern void v_freeze_finish(FILE *fp);
extern void v_freeze_start(FILE *fp);
extern int v_freeze_string(char *string, FILE *fp);
extern void *v_thaw(FILE *fp);
extern int v_thaw_double(FILE *fp, double *value);
extern void v_thaw_err(char *fmt, ...);
extern char *v_thaw_error(void);
extern void v_thaw_expected(char *desc);
extern void *v_thaw_file(char *file);
extern void v_thaw_filename(char *file);
extern void v_thaw_finish(void);
extern int v_thaw_follow(FILE *fp, enum v_tokentype token, char *desc);
extern int v_thaw_getline(void);
extern int v_thaw_int(FILE *fp, int *value);
extern enum v_tokentype v_thaw_peek(FILE *fp);
extern void v_thaw_setline(int num);
extern void v_thaw_start(void);
extern int v_thaw_string(FILE *fp, char **value);
extern enum v_tokentype v_thaw_token(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
