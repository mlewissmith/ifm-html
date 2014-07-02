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
  @brief List functions and macros.
  @ingroup list
*/

#ifndef VARS_LIST_H
#define VARS_LIST_H

#include <vars-scalar.h>

/*! @brief Abort if a pointer is not a list. */
#define VL_CHECK(ptr)          V_CHECK(ptr, vl_check, "LIST")

/*! @brief Return type of a list element. */
#define vl_type(l, n)          vs_type(vl_get(l, n))

/*! @brief Remove and return a string value from a list. */
#define vl_sremove(l, i)       vs_free_sget(vl_remove(l, i))

/*! @brief Insert a string value into a list. */
#define vl_sinsert(l, i, n)    vl_insert(l, i, vs_screate(n))

/*! @brief Insert a string value into a list, keeping it sorted. */
#define vl_sinsort(l, n)       vl_insort(l, vs_cmp, vs_screate(n))

/*! @brief Pop and return a string value from the end of a list. */
#define vl_spop(l)             vs_free_sget(vl_pop(l))

/*! @brief Push a string value onto the end of a list. */
#define vl_spush(l, n)         vl_push(l, vs_screate(n))

/*! @brief Remove and return a string value from the beginning of a list. */
#define vl_sshift(l)           vs_free_sget(vl_shift(l))

/*! @brief Add a string value to the beginning of a list. */
#define vl_sunshift(l, n)      vl_unshift(l, vs_screate(n))

/*! @brief Get a string value from a list element. */
#define vl_sget(l, n)          vs_sget(vl_get(l, n))

/*! @brief Store a string value in a list element. */
#define vl_sstore(l, i, n)     vl_store(l, i, vs_screate(n))

/*! @brief Get the string value from the first element in a list. */
#define vl_shead(l)            vs_sget(vl_head(l))

/*! @brief Get the string value from the last element in a list. */
#define vl_stail(l)            vs_sget(vl_tail(l))

/*! @brief Get a copy of a string value from a list element. */
#define vl_sgetcopy(l, n)      vs_sgetcopy(vl_get(l, n))

/*! @brief Get a reference to a string value from a list element. */
#define vl_sgetref(l, n)       vs_sgetref(vl_get(l, n))

/*! @brief Get a copy of the string value from the first element in a list. */
#define vl_sheadcopy(l)        vs_sgetcopy(vl_head(l))

/*! @brief Get a reference to the string value from the first element in a list. */
#define vl_sheadref(l)         vs_sgetref(vl_head(l))

/*! @brief Get a copy of the string value from the last element in a list. */
#define vl_stailcopy(l)        vs_sgetcopy(vl_tail(l))

/*! @brief Get a reference to the string value from the last element in a list. */
#define vl_stailref(l)         vs_sgetref(vl_tail(l))

/*! @brief Remove and return an integer value from a list. */
#define vl_iremove(l, i)       vs_free_iget(vl_remove(l, i))

/*! @brief Insert an integer value into a list. */
#define vl_iinsert(l, i, n)    vl_insert(l, i, vs_icreate(n))

/*! @brief Insert an integer value into a list, keeping it sorted. */
#define vl_iinsort(l, n)       vl_insort(l, vs_numcmp, vs_icreate(n))

/*! @brief Pop and return an integer value from the end of a list. */
#define vl_ipop(l)             vs_free_iget(vl_pop(l))

/*! @brief Push an integer value onto the end of a list. */
#define vl_ipush(l, n)         vl_push(l, vs_icreate(n))

/*! @brief Remove and return an integer value from the beginning of a list. */
#define vl_ishift(l)           vs_free_iget(vl_shift(l))

/*! @brief Add an integer value to the beginning of a list. */
#define vl_iunshift(l, n)      vl_unshift(l, vs_icreate(n))

/*! @brief Get an integer value from a list element. */
#define vl_iget(l, n)          vs_iget(vl_get(l, n))

/*! @brief Store an integer value in a list element. */
#define vl_istore(l, i, n)     vl_store(l, i, vs_icreate(n))

/*! @brief Get the integer value from the first element in a list. */
#define vl_ihead(l)            vs_iget(vl_head(l))

/*! @brief Get the integer value from the last element in a list. */
#define vl_itail(l)            vs_iget(vl_tail(l))

/*! @brief Remove and return a float value from a list. */
#define vl_fremove(l, i)       vs_free_fget(vl_remove(l, i))

/*! @brief Insert a float value into a list. */
#define vl_finsert(l, i, n)    vl_insert(l, i, vs_fcreate(n))

/*! @brief Insert a float value into a list, keeping it sorted. */
#define vl_finsort(l, n)       vl_insort(l, vs_numcmp, vs_fcreate(n))

/*! @brief Pop and return a float value from the end of a list. */
#define vl_fpop(l)             vs_free_fget(vl_pop(l))

/*! @brief Push a float value onto the end of a list. */
#define vl_fpush(l, n)         vl_push(l, vs_fcreate(n))

/*! @brief Remove and return a float value from the beginning of a list. */
#define vl_fshift(l)           vs_free_fget(vl_shift(l))

/*! @brief Add a float value to the beginning of a list. */
#define vl_funshift(l, n)      vl_unshift(l, vs_fcreate(n))

/*! @brief Get a float value from a list element. */
#define vl_fget(l, n)          vs_fget(vl_get(l, n))

/*! @brief Store a float value in a list element. */
#define vl_fstore(l, i, n)     vl_store(l, i, vs_fcreate(n))

/*! @brief Get a float value from the first element in a list. */
#define vl_fhead(l)            vs_fget(vl_head(l))

/*! @brief Get a float value from the last element in a list. */
#define vl_ftail(l)            vs_fget(vl_tail(l))

/*! @brief Remove and return a double value from a list. */
#define vl_dremove(l, i)       vs_free_dget(vl_remove(l, i))

/*! @brief Insert a double value into a list. */
#define vl_dinsert(l, i, n)    vl_insert(l, i, vs_dcreate(n))

/*! @brief Insert a double value into a list, keeping it sorted. */
#define vl_dinsort(l, n)       vl_insort(l, vs_numcmp, vs_dcreate(n))

/*! @brief Pop and return a double value from the end of a list. */
#define vl_dpop(l)             vs_free_dget(vl_pop(l))

/*! @brief Push a double value onto the end of a list. */
#define vl_dpush(l, n)         vl_push(l, vs_dcreate(n))

/*! @brief Remove and return a double value from the beginning of a list. */
#define vl_dshift(l)           vs_free_dget(vl_shift(l))

/*! @brief Add a double value to the beginning of a list. */
#define vl_dunshift(l, n)      vl_unshift(l, vs_dcreate(n))

/*! @brief Get a double value from a list element. */
#define vl_dget(l, n)          vs_dget(vl_get(l, n))

/*! @brief Store a double value in a list element. */
#define vl_dstore(l, i, n)     vl_store(l, i, vs_dcreate(n))

/*! @brief Get a double value from the first element in a list. */
#define vl_dhead(l)            vs_dget(vl_head(l))

/*! @brief Get a double value from the last element in a list. */
#define vl_dtail(l)            vs_dget(vl_tail(l))

/*! @brief Remove and return a pointer value from a list. */
#define vl_premove(l, i)       vs_free_pget(vl_remove(l, i))

/*! @brief Insert a pointer value into a list. */
#define vl_pinsert(l, i, n)    vl_insert(l, i, vs_pcreate(n))

/*! @brief Insert a pointer value into a list, keeping it sorted. */
#define vl_pinsort(l, cmp, n)  vl_insort(l, cmp, vs_pcreate(n))

/*! @brief Pop and return a pointer value from the end of a list. */
#define vl_ppop(l)             vs_free_pget(vl_pop(l))

/*! @brief Push a pointer value onto the end of a list. */
#define vl_ppush(l, p)         vl_push(l, vs_pcreate((void *) p))

/*! @brief Remove and return a pointer value from the beginning of a list. */
#define vl_pshift(l)           vs_free_pget(vl_shift(l))

/*! @brief Add a pointer value to the beginning of a list. */
#define vl_punshift(l, p)      vl_unshift(l, vs_pcreate((void *) p))

/*! @brief Get a pointer value from a list element. */
#define vl_pget(l, n)          vs_pget(vl_get(l, n))

/*! @brief Store a pointer value in a list element. */
#define vl_pstore(l, i, n)     vl_store(l, i, vs_pcreate(n))

/*! @brief Get a pointer value from the first element in a list. */
#define vl_phead(l)            vs_pget(vl_head(l))

/*! @brief Get a pointer value from the last element in a list. */
#define vl_ptail(l)            vs_pget(vl_tail(l))

/*!
  @brief   Split a string into elements.

  Split a string into fields delimited by any of the characters in \c
  sep. Multiple successive occurrences of delimiter characters are
  ignored. Return a list of string scalars. If \c delim is \c NULL,
  splitting is done on whitespace characters (spaces, tabs, formfeeds and
  newlines).
*/
#define vl_split(s, delim)     vl_split_string(s, delim, NULL, 0)

/*!
  @brief   Split a string into fields.

  Like vl_split(), except treat each occurrence of a delimiter character as
  beginning a new field. This means that multiple successive occurrences of
  such characters will create blank entries in the list.
*/
#define vl_nsplit(s, delim)    vl_split_string(s, delim, NULL, 1)

/*!
  @brief   Split a string into quoted elements.

  Just like vl_split(), except recognize quoted parts of the string.
  Quoted parts are individual substrings that are allowed to contain
  characters from \c delim. The \c quote argument must be either a single
  character, in which case that character serves as both the open- and
  close-quote, or two characters, in which case the first is the open-quote
  and the second is the close-quote.
*/
#define vl_qsplit(s, delim, quote) vl_split_string(s, delim, quote, 0)

/*!
  @brief   Split a string into quoted fields.

  Like vl_qsplit(), but treat delimiter characters in the same way as
  vl_nsplit().
*/
#define vl_qnsplit(s, delim, quote) vl_split_string(s, delim, quote, 1)

/*! @brief Join strings and return a copy of the list. */
#define vl_joincopy(l, delim)  V_STRDUP(vl_join(l, delim))

/*! @brief Search a list for an element. */
#define vl_search(l, key, cmp, match) \
        vl_search_list(l, key, cmp, match, 0)

/*! @brief Search for an integer key using vl_search(). */
#define vl_isearch(l, key, cmp, match) \
        vl_search_list(l, vs_icreate(key), cmp, match, 1)

/*! @brief Search for a float key using vl_search(). */
#define vl_fsearch(l, key, cmp, match) \
        vl_search_list(l, vs_fcreate(key), cmp, match, 1)

/*! @brief Search for a double key using vl_search(). */
#define vl_dsearch(l, key, cmp, match) \
        vl_search_list(l, vs_dcreate(key), cmp, match, 1)

/*! @brief Search for a string key using vl_search(). */
#define vl_ssearch(l, key, cmp, match) \
        vl_search_list(l, vs_screate(key), cmp, match, 1)

/*! @brief Search for a pointer key using vl_search(). */
#define vl_psearch(l, key, cmp, match) \
        vl_search_list(l, vs_pcreate(key), cmp, match, 1)

/*! @brief Create or empty a list. */
#define vl_init(l) if (l == NULL) l = vl_create(); else vl_empty(l)

/*! @brief Iteration scalar value. */
#define vl_iter_val(iter)       v_iter_val(iter)

/*! @brief Iteration scalar integer value. */
#define vl_iter_ival(iter)      v_iter_ival(iter)

/*! @brief Iteration scalar float value. */
#define vl_iter_fval(iter)      v_iter_fval(iter)

/*! @brief Iteration scalar double value. */
#define vl_iter_dval(iter)      v_iter_dval(iter)

/*! @brief Iteration scalar string value. */
#define vl_iter_sval(iter)      v_iter_sval(iter)

/*! @brief Iteration scalar string value reference. */
#define vl_iter_svalref(iter)   v_iter_svalref(iter)

/*! @brief Iteration scalar string value copy. */
#define vl_iter_svalcopy(iter)  v_iter_svalcopy(iter)

/*! @brief Iteration scalar pointer value. */
#define vl_iter_pval(iter)      v_iter_pval(iter)

/*! @brief List search comparison types. */
enum v_cmptype {
    V_CMP_NOTFOUND = -1,        /*!< Not found in list */
    V_CMP_SMALLER = -2,         /*!< Smaller than smallest element */
    V_CMP_GREATER = -3,         /*!< Greater than largest element */
};

/*! @brief List type. */
typedef struct v_list vlist;

#ifdef __cplusplus
extern "C" {
#endif

extern void vl_append(vlist *l, vlist *a);
extern void vl_apply(vlist *l, void (*applyfunc)(vscalar *s));
extern void vl_break(vlist *l);
extern int vl_check(void *ptr);
extern vlist *vl_concat(vlist *a, vlist *b);
extern vlist *vl_copy(vlist *l);
extern vlist *vl_create(void);
extern vlist *vl_create_size(unsigned size);
extern vtype *vl_declare(void);
extern int vl_defined(vlist *l, int num);
extern void vl_destroy(vlist *l);
extern void vl_empty(vlist *l);
extern vlist *vl_filltext(char *text, int width);
extern int vl_freeze(vlist *l, FILE *fp);
extern vscalar *vl_get(vlist *l, int num);
extern vscalar *vl_head(vlist *l);
extern int vl_insert(vlist *l, int num, vscalar *s);
extern int vl_insort(vlist *l, int (*compare)(vscalar **s1, vscalar **s2),
                     vscalar *s);
extern char *vl_join(vlist *l, char *delim);
extern char *vl_join_text(vlist *l, char *endings);
extern int vl_length(vlist *l);
extern vlist *vl_list(int type, ...);
extern vlist *vl_makelist(char **list);
extern vlist *vl_map(vlist *l, vscalar *(*mapfunc)(vscalar *s));
extern vlist *vl_match(vlist *l, int (*matchfunc)(vscalar *s));
extern int vl_next(viter *iter);
extern vscalar *vl_pop(vlist *l);
extern void vl_print(vlist *l, FILE *fp);
extern int vl_push(vlist *l, vscalar *s);
extern vlist *vl_read(FILE *fp);
extern vscalar *vl_remove(vlist *l, int num);
extern vlist *vl_reverse(vlist *l);
extern int vl_search_list(vlist *l, vscalar *s, int (*compare)(vscalar **s1,
                          vscalar **s2), int match, int destroy);
extern vscalar *vl_shift(vlist *l);
extern void vl_sort(vlist *l, int (*compare)(vscalar **s1, vscalar **s2));
extern vlist *vl_splice(vlist *l, int offset, int length, vlist *r);
extern vlist *vl_split_string(char *string, char *sep, char *quotes,
                              int nullfields);
extern void vl_store(vlist *l, int num, vscalar *s);
extern vscalar *vl_tail(vlist *l);
extern vlist *vl_thaw(FILE *fp);
extern int vl_traverse(vlist *l, int (*func)(void *ptr));
extern void vl_undef(vlist *l, int num);
extern void vl_unshift(vlist *l, vscalar *s);
extern int vl_write(vlist *l, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
