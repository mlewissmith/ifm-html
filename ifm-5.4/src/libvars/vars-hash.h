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
  @brief Hash functions and macros.
  @ingroup hash
*/

#ifndef VARS_HASH_H
#define VARS_HASH_H

#include <vars-list.h>
#include <vars-utils.h>

/*! @brief Abort if a pointer is not a hash. */
#define VH_CHECK(ptr)           V_CHECK(ptr, vh_check, "HASH")

/*! @brief Return type of a hash key. */
#define vh_type(h, key)         vs_type(vh_get(h, key))

/*! @brief Get the string value of a hash key. */
#define vh_sget(h, key)		vs_sget(vh_get(h, key))

/*! @brief Get a copy of the string value of a hash key. */
#define vh_sgetcopy(h, key)     vs_sgetcopy(vh_get(h, key))

/*! @brief Get a reference to the string value of a hash key. */
#define vh_sgetref(h, key)	vs_sgetref(vh_get(h, key))

/*! @brief Set the string value of a hash key. */
#define vh_sstore(h, key, n)	vh_store(h, key, vs_screate(n))

/*! @brief Get the float value of a hash key. */
#define vh_fget(h, key)		vs_fget(vh_get(h, key))

/*! @brief Set the float value of a hash key. */
#define vh_fstore(h, key, n)	vh_store(h, key, vs_fcreate(n))

/*! @brief Get the double value of a hash key. */
#define vh_dget(h, key)		vs_dget(vh_get(h, key))

/*! @brief Set the double value of a hash key. */
#define vh_dstore(h, key, n)	vh_store(h, key, vs_dcreate(n))

/*! @brief Get the integer value of a hash key. */
#define vh_iget(h, key)         vs_iget(vh_get(h, key))

/*! @brief Set the integer value of a hash key. */
#define vh_istore(h, key, n)    vh_store(h, key, vs_icreate(n))

/*! @brief Get the pointer value of a hash key. */
#define vh_pget(h, key)		vs_pget(vh_get(h, key))

/*! @brief Set the pointer value of a hash key. */
#define vh_pstore(h, key, p)	vh_store(h, key, vs_pcreate((void *) p))

/*! @brief Return hash of unique abbreviations for a list of words. */
#define vh_abbrev(words)        vh_abbrev_table(words, 0)

/*! @brief Return case-insensitive hash of unique abbreviations for a list of words. */
#define vh_abbrev_nocase(words) vh_abbrev_table(words, 1)

/*! @brief Build a hash key from a pointer using a buffer. */
#define vh_pkey_buf(ptr, buf)   v_hexstring(ptr, buf)

/*! @brief Build a hash key from a pointer. */
#define vh_pkey(ptr)            vh_pkey_buf(ptr, vh_keybuf)

/*! @brief Build a hash key from an integer using a buffer. */
#define vh_ikey_buf(num, buf)   (sprintf(buf, "%d", num), buf)

/*! @brief Build a hash key from an integer. */
#define vh_ikey(num)            vh_ikey_buf(num, vh_keybuf)

/*! @brief Build a hash key from a float using a buffer. */
#define vh_fkey_buf(num, buf)   (sprintf(buf, "%g", num), buf)

/*! @brief Build a hash key from a float. */
#define vh_fkey(num)            vh_fkey_buf(num, vh_keybuf)

/*! @brief Build a hash key from a double using a buffer. */
#define vh_dkey_buf(num, buf)   vh_fkey_buf(num, buf)

/*! @brief Build a hash key from a double. */
#define vh_dkey(num)            vh_fkey(num)

/*! @brief Create or empty a hash. */
#define vh_init(h) if (h == NULL) h = vh_create(); else vh_empty(h)

/*! @brief Iteration key value. */
#define vh_iter_key(iter)       iter.sval[0]

/*! @brief Iteration scalar value. */
#define vh_iter_val(iter)       v_iter_val(iter)

/*! @brief Iteration scalar integer value. */
#define vh_iter_ival(iter)      v_iter_ival(iter)

/*! @brief Iteration scalar float value. */
#define vh_iter_fval(iter)      v_iter_fval(iter)

/*! @brief Iteration scalar double value. */
#define vh_iter_dval(iter)      v_iter_dval(iter)

/*! @brief Iteration scalar string value. */
#define vh_iter_sval(iter)      v_iter_sval(iter)

/*! @brief Iteration scalar string value reference. */
#define vh_iter_svalref(iter)   v_iter_svalref(iter)

/*! @brief Iteration scalar string value copy. */
#define vh_iter_svalcopy(iter)  v_iter_svalcopy(iter)

/*! @brief Iteration scalar pointer value. */
#define vh_iter_pval(iter)      v_iter_pval(iter)

/*! @brief Add array to hash if it's not there. */
#define vh_add_array(h, key)    vh_add_object(h, key, "A")

/*! @brief Add buffer to hash if it's not there. */
#define vh_add_buffer(h, key)   vh_add_object(h, key, "B")

/*! @brief Add database to hash if it's not there. */
#define vh_add_database(h, key) vh_add_object(h, key, "D")

/*! @brief Add function to hash if it's not there. */
#define vh_add_func(h, key)     vh_add_object(h, key, "F")

/*! @brief Add graph to hash if it's not there. */
#define vh_add_graph(h, key)    vh_add_object(h, key, "G")

/*! @brief Add hash to hash if it's not there. */
#define vh_add_hash(h, key)     vh_add_object(h, key, "H")

/*! @brief Add list to hash if it's not there. */
#define vh_add_list(h, key)     vh_add_object(h, key, "L")

/*! @brief Add matrix to hash if it's not there. */
#define vh_add_matrix(h, key)   vh_add_object(h, key, "M")

/*! @brief Add parser to hash if it's not there. */
#define vh_add_parser(h, key)   vh_add_object(h, key, "P")

/*! @brief Add queue to hash if it's not there. */
#define vh_add_queue(h, key)    vh_add_object(h, key, "Q")

/*! @brief Hash type. */
typedef struct v_hash vhash;

#define VH_NOREUSE(cmd) do {                    \
    vh_reuse_keys = 0; cmd; vh_reuse_keys = 1;  \
} while (0)

extern int vh_reuse_keys;

#ifdef __cplusplus
extern "C" {
#endif

extern char vh_keybuf[];
extern vhash *vh_abbrev_table(vlist *words, int nocase);
extern void *vh_add_object(vhash *h, char *key, char *code);
extern int vh_append(vhash *h, vhash *a, int replace);
extern int vh_check(void *ptr);
extern vhash *vh_copy(vhash *h);
extern vhash *vh_create(void);
extern vhash *vh_create_size(unsigned size);
extern vtype *vh_declare(void);
extern int vh_defined(vhash *h, char *key);
extern int vh_delete(vhash *h, char *key);
extern void vh_destroy(vhash *h);
extern void vh_empty(vhash *h);
extern int vh_entry_count(vhash *h);
extern int vh_exists(vhash *h, char *key);
extern int vh_freeze(vhash *h, FILE *fp);
extern vscalar *vh_get(vhash *h, char *key);
extern vhash *vh_hash(char *key, ...);
extern int *vh_hashinfo(vhash *h);
extern char *vh_intern(char *key);
extern int vh_interned(char *key);
extern vlist *vh_keys(vhash *h);
extern vhash *vh_makehash(char **list);
extern int vh_next(viter *iter);
extern void vh_print(vhash *h, FILE *fp);
extern vhash *vh_read(FILE *fp);
extern void vh_rehash(unsigned count);
extern void vh_store(vhash *h, char *key, vscalar *val);
extern vhash *vh_thaw(FILE *fp);
extern int vh_traverse(vhash *h, int (*func)(void *ptr));
extern void vh_undef(vhash *h, char *key);
extern vlist *vh_values(vhash *h);
extern int vh_write(vhash *h, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
