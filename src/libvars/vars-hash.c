/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup hash Hashes
  @ingroup types

  A hash (or hash table, or associative array) is a set of key-value pairs.
  Each key is a string, and its corresponding value is a scalar.
*/

/*!
  @defgroup hash_create Creating and destroying hashes
  @ingroup hash
*/

/*!
  @defgroup hash_access Accessing hash elements
  @ingroup hash
*/

/*!
  @defgroup hash_modify Modifying hash elements
  @ingroup hash
*/

/*!
  @defgroup hash_convert Converting hashes to lists
  @ingroup hash
*/

/*!
  @defgroup hash_misc Other hash functions
  @ingroup hash
*/

/*!
  @defgroup hash_intern Internal hash functions
  @ingroup hash

  Hash tables are represented by an internal array of hash 'buckets'.  Each
  table entry hashes to one of these buckets.  Multiple entries that hash
  to the same bucket are strung together in a list -- this is known as a
  'collision'.  The number of collisions increases as the hash table fills
  up, and if nothing is done this will slow down access times, so at
  certain points a hash table is 'rehashed' -- all its contents are moved
  to a bigger table to reduce collisions.

  Rehashing occurs in the vh_store() function, when a threshold number of
  collisions is detected while adding a new table entry.  The next biggest
  table size is chosen from an internal list of 'good' table sizes -- these
  are the primes closest to the powers of 2 from 2^3 to 2^20, for reasons
  of efficiency.  Once the maximum table size is reached, no more rehashing
  is done.

  If you know you're going to add a lot of entries to a hash, you may as
  well start with a big table size, using vh_create_size(), and save the
  overhead of rehashing.

  Hash key strings are stored in a special internal hash table, so that
  they are only allocated once.  The function vh_intern() is used
  internally to store them.  This function is also useful in programs to
  store strings that are used in many different places, with the caveat
  that you must never deallocate them.  It's safe to store interned strings
  in scalars; vs_destroy() knows not to destroy them.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "vars-config.h"
#include "vars-hash.h"
#include "vars-macros.h"
#include "vars-memory.h"
#include "vars-yaml.h"

/* Calculate a hash value */
#define VH_HASHVAL(size, key, value) do {       \
    char *_kp = key;                            \
    for (value = 0; *_kp != '\0'; _kp++)        \
        value = *_kp + 31 * value;              \
    value %= size;                              \
} while (0)

/* Calculate a hash value */
#define VH_HASH(hash, key, value) \
        VH_HASHVAL(hash->tablesize, key, value)

/* Find entry in hash table */
#define VH_FIND(hash, key, value, entry) do {   \
    for (entry = hash->table[value];            \
         entry != NULL;                         \
         entry = entry->next)                   \
        if (V_STREQ(key, entry->key))           \
            break;                              \
} while (0)

/* Find an entry with collision counting */
#define VH_FIND_COUNT(hash, key, value, entry, count) do {      \
    for (entry = hash->table[value], count = 0;                 \
         entry != NULL;                                         \
         entry = entry->next, count++)                          \
        if (V_STREQ(key, entry->key))                           \
            break;                                              \
} while (0)

/* Iterate over all entries */
#define VH_ITERATE(hash, hp, stmts) do {                        \
    int _i;                                                     \
    for (_i = 0; _i < hash->tablesize; _i++) {                  \
        for (hp = hash->table[_i]; hp != NULL; hp = hp->next) { \
            stmts;                                              \
        }                                                       \
    }                                                           \
} while (0)

/* Get the next highest hash table size */
#define VH_NEXTSIZE(num, next) do {                     \
    int _i = 0;                                         \
    next = primes[0];                                   \
    while (primes[_i + 1] != 0 && num > primes[_i++])   \
        next = primes[_i];                              \
} while (0)

/* See if a string looks like a pointer value */
#define VH_PSTRING(str) (str[0] == '0' && str[1] == 'x')

/* Type definition */
struct v_hash {
    struct v_header id;         /* Type marker */
    unsigned tablesize;         /* Hash table size */
    struct v_table **table;     /* Hash table */
};

/* Hash table elements */
typedef struct v_table {
    char *key;                  /* Key value */
    struct v_scalar *val;       /* Scalar value */
    struct v_table *next;       /* Link */
} vtable;

/* Type variable */
vtype *vhash_type = NULL;

/* List of primes closest to powers of 2 */
static int primes[] = {
    7, 17, 31, 61, 127, 257, 509, 1021, 2053, 4099, 8191, 16381, 32771,
    65537, 131101, 262147, 524309, 1048583, 0
};

/* Rehashing collision threshold */
static unsigned vh_collision = 10;

/* Hash key table */
static vhash *hashkeys = NULL;

/* Whether interning a hash key string */
static int vh_interning = 0;

/* Buffer for hash key conversions */
char vh_keybuf[V_HEXSTRING_SIZE];

/* Internal functions */
static vtable **vh_entries(vhash *h);
static int vh_entries_cmp(vtable **t1, vtable **t2);
static int vh_xmldump(vhash *h, FILE *fp);
static int vh_yamldump(vhash *h, FILE *fp);

/*!
  @brief   Create abbreviation table from word list.
  @ingroup hash_misc
  @param   words List of words.
  @param   nocase Whether to ignore case.
  @return  Hash of abbreviations.
  @see     vh_abbrev(), vh_abbrev_nocase()

  Given a list of words, return a hash of the unique abbreviations of those
  words.  If a character string is an abbreviation of a word in the list,
  it appears as a key in the hash.  If it a unique abbreviation, the
  corresponding value is the word itself.  If not, the corresponding value
  is undefined.  Words in the original list always map to themselves, even
  if they are an abbreviation of a longer word.

  If \c nocase is set, make the hash keys lower-case.  Hash values are
  still words of the original list.  Ambiguities will arise if two words in
  the list map to the same lower-case word.
*/
vhash *
vh_abbrev_table(vlist *words, int nocase)
{
    char *word, *cp, buf[100];
    vhash *table;
    viter iter;
    int len;

    VL_CHECK(words);
    table = vh_create();

    /* Add abbreviations */
    v_iterate(words, iter) {
        word = vl_iter_svalref(iter);
        if ((len = strlen(word)) == 0)
            continue;

        strcpy(buf, word);
        if (nocase)
            for (cp = buf; *cp != '\0'; cp++)
                *cp = tolower(*cp);

        while (len > 0) {
            buf[len--] = '\0';
            if (vh_exists(table, buf))
                vh_undef(table, buf);
            else
                vh_sstore(table, buf, word);
        }
    }

    /* Add the words themselves */
    v_iterate(words, iter) {
        word = vl_iter_svalref(iter);
        strcpy(buf, word);

        if (nocase)
            for (cp = buf; *cp != '\0'; cp++)
                *cp = tolower(*cp);

        vh_sstore(table, buf, word);
    }

    return table;
}

/*!
  @brief   Add object to hash, or create it if it's not there.
  @ingroup hash_modify

  @param   h Hash to add to.
  @param   key Hash key.
  @param   code Object type code.

  @return  Object added.

  If the object in the hash key already exists, it is just returned.

  @see     vh_add_array(), vh_add_buffer(), vh_add_database(),
           vh_add_func(), vh_add_graph(), vh_add_hash(), vh_add_list(),
           vh_add_matrix(), vh_add_parser(), vh_add_queue().
*/
void *
vh_add_object(vhash *h, char *key, char *code)
{
    void *obj;

    VH_CHECK(h);

    if ((obj = vh_pget(h, key)) == NULL) {
        obj = v_create_object(code);
        vh_pstore(h, key, obj);
    }

    if (v_find(code) != v_type(obj))
        v_fatal("vh_add_object(): object type mismatch");

    return obj;
}

/*!
  @brief   Append hash entries to a hash.
  @ingroup hash_modify
  @param   h Hash to append to.
  @param   a Hash to append entries from.
  @param   replace Whether to replace existing entries.
  @return  No. of entries appended.
*/
int
vh_append(vhash *h, vhash *a, int replace)
{
    int count = 0;
    vscalar *elt;
    viter iter;
    char *key;

    VH_CHECK(h);
    VH_CHECK(a);

    v_iterate(a, iter) {
        key = vh_iter_key(iter);
        elt = vh_iter_val(iter);
        if (replace || !vh_exists(h, key)) {
            vh_store(h, key, vs_copy(elt));
            count++;
        }
    }

    return count;
}

/* Check if pointer is a hash */
int
vh_check(void *ptr)
{
    return (ptr != NULL && v_type(ptr) == vhash_type);
}

/*!
  @brief   Return a copy of a hash table.
  @ingroup hash_create
  @param   h Hash.
  @return  Copy of the hash.
*/
vhash *
vh_copy(vhash *h)
{
    vtable *hp;
    vhash *hc;

    VH_CHECK(h);

    hc = vh_create_size(h->tablesize);
    VH_ITERATE(h, hp, vh_store(hc, hp->key, vs_copy(hp->val)));

    return hc;
}

/*!
  @brief   Return a new hash.
  @ingroup hash_create
  @return  New hash.
*/
vhash *
vh_create(void)
{
    return vh_create_size(0);
}

/*!
  @brief   Return a new hash with a given size.
  @ingroup hash_create
  @param   size Initial size.
  @return  New hash.
  @see     vh_create()
*/
vhash *
vh_create_size(unsigned size)
{
    static vheader *id = NULL;
    vhash *h;
    int i;

    if (id == NULL) {
        vh_declare();
        id = v_header(vhash_type);
    }

    h = V_ALLOC(vhash, 1);

    VH_NEXTSIZE(size, h->tablesize);

    h->id = *id;
    h->table = V_ALLOC(vtable *, h->tablesize);

    for (i = 0; i < h->tablesize; i++)
	h->table[i] = NULL;

    return h;
}

/* Declare hash type */
vtype *
vh_declare(void)
{
    if (vhash_type == NULL) {
        vhash_type = v_create("HASH", "H");
	v_create_func(vhash_type, (void *(*)()) vh_create);
        v_copy_func(vhash_type, (void *(*)()) vh_copy);
        v_next_func(vhash_type, vh_next);
        v_read_func(vhash_type, (void *(*)()) vh_read);
        v_write_func(vhash_type, vh_write);
        v_freeze_func(vhash_type, vh_freeze);
        v_thaw_func(vhash_type, (void *(*)()) vh_thaw);
        v_print_func(vhash_type, vh_print);
        v_xmldump_func(vhash_type, vh_xmldump);
        v_yamldump_func(vhash_type, vh_yamldump);
        v_destroy_func(vhash_type, vh_destroy);
        v_traverse_func(vhash_type, vh_traverse);
    }

    return vhash_type;
}

/*!
  @brief   Return whether a hash entry is defined.
  @ingroup hash_access
  @param   h Hash.
  @param   key Entry to check.
  @return  Yes or no.
*/
int
vh_defined(vhash *h, char *key)
{
    unsigned hashval;
    vtable *hp;

    VH_CHECK(h);
    VH_HASH(h, key, hashval);
    VH_FIND(h, key, hashval, hp);

    if (hp != NULL && hp->val != NULL)
        return vs_defined(hp->val);

    return 0;
}

/*!
  @brief   Delete a key-value pair from a hash.
  @ingroup hash_modify
  @param   h Hash.
  @param   key Entry to delete.
  @return  Whether anything was deleted.

  Differs from vh_undef() in that the key is also deleted (i.e. vh_exists()
  will return false).
*/
int
vh_delete(vhash *h, char *key)
{
    vtable *hp, *hpprev = NULL;
    unsigned hval;

    VH_CHECK(h);
    VH_HASH(h, key, hval);

    for (hp = h->table[hval]; hp != NULL; hp = hp->next) {
	if (V_STREQ(key, hp->key)) {
	    if (hpprev == NULL)
		h->table[hval] = hp->next;
	    else
		hpprev->next = hp->next;

            if (VH_PSTRING(hp->key))
                V_DEALLOC(hp->key);

	    if (hp->val != NULL)
		vs_destroy(hp->val);

	    V_DEALLOC(hp);
	    return 1;
	} else {
	    hpprev = hp;
	}
    }

    return 0;
}

/*!
  @brief   Deallocate a hash table and its contents.
  @ingroup hash_create
  @param   h Hash.
*/
void
vh_destroy(vhash *h)
{
    VH_CHECK(h);

    vh_empty(h);
    V_DEALLOC(h->table);
    V_DEALLOC(h);
}

/*!
  @brief   Empty a hash.
  @ingroup hash_modify
  @param   h Hash.
*/
void
vh_empty(vhash *h)
{
    vtable *hp, *hpnext;
    int i;

    VH_CHECK(h);

    for (i = 0; i < h->tablesize; i++) {
	for (hp = h->table[i]; hp != NULL; hp = hpnext) {
	    hpnext = hp->next;

            if (VH_PSTRING(hp->key))
                V_DEALLOC(hp->key);

	    if (hp->val != NULL)
		vs_destroy(hp->val);

	    V_DEALLOC(hp);
	}

        h->table[i] = NULL;
    }
}

/* Return sorted list of hash table entries */
static vtable **
vh_entries(vhash *h)
{
    vtable **entries = NULL;
    int size = 0, count = 0;
    vtable *hp;

    VH_ITERATE(h, hp, size++);
    entries = V_ALLOC(vtable *, size + 1);

    VH_ITERATE(h, hp, entries[count++] = hp);
    entries[count] = NULL;

    qsort(entries, (size_t) size, sizeof(vtable *),
          (int (*)(const void *, const void *)) vh_entries_cmp);

    return entries;
}

/* Table entry sort function */
static int
vh_entries_cmp(vtable **t1, vtable **t2)
{
    char *k1 = (*t1)->key;
    char *k2 = (*t2)->key;
    return strcmp(k1, k2);
}

/*!
  @brief   Return the no. of entries in a hash.
  @ingroup hash_misc
  @param   h Hash.
  @return  No. of entries.
*/
int
vh_entry_count(vhash *h)
{
    int size = 0;
    vtable *hp;

    VH_CHECK(h);
    VH_ITERATE(h, hp, size++);

    return size;
}

/*!
  @brief   Return whether a hash entry exists.
  @ingroup hash_access
  @param   h Hash.
  @param   key Entry to check.
  @return  Yes or no.
*/
int
vh_exists(vhash *h, char *key)
{
    unsigned hashval;
    vtable *hp;

    VH_CHECK(h);
    VH_HASH(h, key, hashval);
    VH_FIND(h, key, hashval, hp);

    return (hp != NULL);
}

/* Freeze contents of a hash */
int
vh_freeze(vhash *h, FILE *fp)
{
    vtable **entries;
    int i;

    VH_CHECK(h);

    v_freeze_start(fp);

    fputs("{\n", fp);
    v_push_indent();

    entries = vh_entries(h);
    for (i = 0; entries[i] != NULL; i++) {
        v_indent(fp);
        if (!v_freeze_string(entries[i]->key, fp))
            return 0;

        fputs(" = ", fp);
        if (!vs_freeze(entries[i]->val, fp))
            return 0;

        fputs(",\n", fp);
    }

    V_DEALLOC(entries);

    v_pop_indent();
    v_indent(fp);
    fputc('}', fp);

    v_freeze_finish(fp);

    return 1;
}

/*!
  @brief   Return a value from a hash.
  @ingroup hash_access
  @param   h Hash.
  @param   key Entry to get.
  @return  Scalar value.
  @retval  NULL if not defined.
*/
vscalar *
vh_get(vhash *h, char *key)
{
    unsigned hashval;
    vtable *hp;

    VH_CHECK(h);
    VH_HASH(h, key, hashval);
    VH_FIND(h, key, hashval, hp);

    return (hp == NULL ? NULL : hp->val);
}

/*!
  @brief   Build a hash from a list of arguments.
  @ingroup hash_create
  @return  New hash.

  Create and return a hash containing a list of given key-value pairs.
  Each hash entry is specified by three parameters: a <tt>char *</tt> key,
  an \c int type, and a value depending on its type.  The type can be one
  of V_TYPE_INT, V_TYPE_FLOAT, V_TYPE_DOUBLE, V_TYPE_STRING, V_TYPE_POINTER
  or V_TYPE_UNDEF.  The type of the following value should correspond to
  this, except in the case of V_TYPE_UNDEF, where the following value
  should be omitted.  The list of key-value pairs should be terminated by a
  \c NULL key.  If a value has the wrong type, or no terminator is
  supplied, chaos will surely result.
*/
vhash *
vh_hash(char *key, ...)
{
    char buf[100];
    va_list ap;
    vhash *h;
    int type;

    h = vh_create();
    va_start(ap, key);

    while (key != NULL) {
        strcpy(buf, key);
        type = va_arg(ap, int);

        switch (type) {
        case V_TYPE_INT:
            vh_istore(h, buf, va_arg(ap, int));
            break;
        case V_TYPE_FLOAT:
            /* Read as double */
            vh_fstore(h, buf, va_arg(ap, double));
            break;
        case V_TYPE_DOUBLE:
            vh_dstore(h, buf, va_arg(ap, double));
            break;
        case V_TYPE_STRING:
            vh_sstore(h, buf, va_arg(ap, char *));
            break;
        case V_TYPE_POINTER:
            vh_pstore(h, buf, va_arg(ap, void *));
            break;
        case V_TYPE_UNDEF:
            vh_store(h, buf, vs_create(V_TYPE_UNDEF));
            break;
        default:
            v_fatal("vh_hash(): invalid scalar type in arg list");
        }

        key = va_arg(ap, char *);
    }

    va_end(ap);
    return h;
}

/*!
  @brief   Return some stats on a hash.
  @ingroup hash_intern
  @param   h Hash.
  @return  Stats array.

  Entries in the array are:
  @verbatim
  0 Total no. of hash buckets
  1 No. of hash buckets used
  2 Total no. of entries in the hash
  3 Largest no. of entries in a hash bucket
  @endverbatim
*/
int *
vh_hashinfo(vhash *h)
{
    static int sizes[4];
    vtable *hp;
    int i, ncol;

    VH_CHECK(h);

    for (i = 0; i < 4; i++)
	sizes[i] = 0;

    sizes[0] = h->tablesize;

    for (i = 0; i < h->tablesize; i++) {
	if (h->table[i] != NULL)
	    sizes[1]++;

	for (hp = h->table[i], ncol = 0; hp != NULL; hp = hp->next, ncol++)
	    sizes[2]++;

	if (ncol > sizes[3])
	    sizes[3] = ncol;
    }

    return sizes;
}

/*!
  @brief   Internalize a hash key string.
  @ingroup hash_intern
  @param   key Key string.
  @return  Internalized string.
  @retval  NULL if key is @c NULL.

  If the key string has not been seen before, internalize it by copying and
  return the copy.  If it has, just return it.

  Strings allocated using this function will never be destroyed.  You can
  use vh_interned() to check whether a string has been interned.

  @warning Don't deallocate the returned string!
*/
char *
vh_intern(char *key)
{
    unsigned hashval;
    vtable *hp;

    /* Special case */
    if (key == NULL)
        return NULL;

    /* Initialize */
    if (hashkeys == NULL)
        hashkeys = vh_create();

    /* If key allocated already, return it */
    VH_HASH(hashkeys, key, hashval);
    VH_FIND(hashkeys, key, hashval, hp);
    if (hp != NULL)
        return hp->key;

    /* Add key */
    vh_interning = 1;
    vh_store(hashkeys, key, NULL);
    vh_interning = 0;

    /* Find pointer to key in table */
    VH_HASH(hashkeys, key, hashval);
    VH_FIND(hashkeys, key, hashval, hp);

    return hp->key;
}

/*!
  @brief   Check if a string has been interned by vh_intern().
  @ingroup hash_intern
  @param   key Key string.
  @return  Yes or no.

  If the key string has not been seen before, internalize it by copying and
  return the copy.  If it has, just return it.

  Strings allocated using this function will never be destroyed.

  @warning Don't deallocate the returned string!
*/
int
vh_interned(char *key)
{
    vtable *hp = NULL;
    unsigned hashval;

    if (key != NULL && hashkeys != NULL) {
        VH_HASH(hashkeys, key, hashval);
        VH_FIND(hashkeys, key, hashval, hp);
    }

    return (hp != NULL);
}

/*!
  @brief   Return a sorted list of keys of a hash.
  @ingroup hash_convert
  @param   h Hash.
  @return  List of keys.
*/
vlist *
vh_keys(vhash *h)
{
    vtable *hp;
    vlist *l;

    VH_CHECK(h);

    l = vl_create();
    VH_ITERATE(h, hp, vl_spush(l, hp->key));
    vl_sort(l, NULL);

    return l;
}

/*!
  @brief   Build a hash from a NULL-terminated list of strings.
  @ingroup hash_create
  @param   list List of strings.
  @return  New hash.

  Consecutive strings represent a single key-value pair.  If there are an
  odd number of strings, then the last key-value pair has a value of
  V_UNDEF.  The list of strings should terminate with \c NULL.
*/
vhash *
vh_makehash(char **list)
{
    char *key;
    vhash *h;

    h = vh_create();

    while (list != NULL && *list != NULL) {
	key = *list++;
        if (*list != NULL)
            vh_sstore(h, key, *list++);
        else
            vh_store(h, key, vs_create(V_TYPE_UNDEF));
    }

    return h;
}

/*!
  @brief   Iterate over a hash.
  @ingroup hash_access
  @param   iter Iterator.
  @return  Whether to continue.
  @see     v_iter(), v_next(), v_iterate()

  Iterate over the key-value pairs in a hash, in an apparently random
  order.  Returns whether there are any more entries.  The iterator must
  have been initialized via v_iter() with a hash object.

  After this call, the iterator structure will contain data on the next
  hash entry, which you can access using vh_iter_key() and vh_iter_val().
*/
int
vh_next(viter *iter)
{
    vhash *h = iter->object;

    VH_CHECK(h);

    while (iter->ipos < h->tablesize) {
	if (iter->ppos == NULL) {
	    iter->ppos = h->table[iter->ipos];
	    if (iter->ppos == NULL)
		iter->ipos++;
	} else {
            vtable *t = iter->ppos;

	    if ((iter->ppos = t->next) == NULL)
		iter->ipos++;

            iter->sval[0] = t->key;
            iter->pval[0] = t->val;

	    return 1;
	}
    }

    return 0;
}

/* Print contents of a hash */
void
vh_print(vhash *h, FILE *fp)
{
    vtable **entries;
    int i;

    VH_CHECK(h);

    v_print_start();
    v_push_indent();

    v_print_type(vhash_type, h, fp);

    entries = vh_entries(h);
    for (i = 0; entries[i] != NULL; i++) {
        v_indent(fp);
        fprintf(fp, "%s => ", entries[i]->key);
        v_print(entries[i]->val, fp);
    }

    V_DEALLOC(entries);

    v_pop_indent();
    v_print_finish();
}

/* Read hash from a stream */
vhash *
vh_read(FILE *fp)
{
    vscalar *s;
    int num, i;
    char *key;
    vhash *h;

    /* Read no. of entries */
    if (!v_read_long(&num, fp))
        return NULL;

    h = vh_create_size(num);

    /* Read key-value pairs */
    for (i = 0; i < num; i++) {
        /* Read key */
        if ((key = v_read_string(fp)) != NULL)
            key = strdup(key);
        else
            return NULL;

        /* Read value and add entry */
        if ((s = vs_read(fp)) != NULL)
            vh_store(h, key, s);

        free(key);

        if (s == NULL) {
            v_destroy(h);
            return NULL;
        }
    }

    return h;
}

/*!
  @brief   Set the rehashing collision threshold.
  @ingroup hash_intern
  @param   count No. of collisions before rehashing.

  Set the rehashing threshold.  Default is 10.  Rehashing will occur when
  this many collisions are detected by vh_store().  A value of zero will
  turn rehashing off completely.
*/
void
vh_rehash(unsigned count)
{
    vh_collision = count;
}

/*!
  @brief   Store a key-value pair in a hash.
  @ingroup hash_modify
  @param   h Hash.
  @param   key Entry to set.
  @param   val Value to set it to.
*/
void
vh_store(vhash *h, char *key, vscalar *val)
{
    vtable *hp, *hpnext, **table;
    static unsigned maxsize = 0;
    int count, size, i;
    unsigned hashval;

    if (h != hashkeys) {
        VH_CHECK(h);
        VS_CHECK(val);
    }

    /* Find entry */
    VH_HASH(h, key, hashval);
    VH_FIND_COUNT(h, key, hashval, hp, count);

    /* Add new entry if it doesn't exist */
    if (hp == NULL) {
        /* Get or allocate key string */
        if (vh_interning || VH_PSTRING(key))
            key = V_STRDUP(key);
        else
            key = vh_intern(key);

        /* Add the entry */
	hp = V_ALLOC(vtable, 1);
	hp->key = key;
	hp->val = NULL;
	hp->next = h->table[hashval];
	h->table[hashval] = hp;
    }

    /* Add new value */
    if (hp->val != NULL && hp->val != val)
	vs_destroy(hp->val);

    hp->val = val;

    /* Rehash not required if collision threshold not reached */
    if (vh_collision == 0 || count < vh_collision)
        return;

    /* Can't rehash if at maximum size */
    if (h->tablesize == maxsize)
        return;

    VH_NEXTSIZE(h->tablesize + 1, size);
    if (size == h->tablesize) {
        maxsize = size;
        return;
    }

    /* Rehash required and possible -- allocate new table */
    table = V_ALLOC(vtable *, size);
    for (i = 0; i < size; i++)
	table[i] = NULL;

    /* Transfer old entries to new table */
    for (i = 0; i < h->tablesize; i++) {
	for (hp = h->table[i]; hp != NULL; hp = hpnext) {
            hpnext = hp->next;
            VH_HASHVAL(size, hp->key, hashval);
            hp->next = table[hashval];
            table[hashval] = hp;
        }
    }

    /* Dispose of old table */
    V_DEALLOC(h->table);

    /* Install new one */
    h->table = table;
    h->tablesize = size;
}

/* Thaw a hash from file */
vhash *
vh_thaw(FILE *fp)
{
    vscalar *s;
    int token;
    char *key;
    vhash *h;

    v_thaw_start();

    h = vh_create();

    if (!v_thaw_follow(fp, '{', "open-brace"))
        goto fail;

    while (1) {
        if (v_thaw_peek(fp) == '}') {
            v_thaw_token(fp);
            break;
        }

        if (!v_thaw_follow(fp, V_TOKEN_STRING, "hash key string"))
            goto fail;

        key = V_STRDUP(v_thaw_svalue);

        if (!v_thaw_follow(fp, '=', "'='"))
            goto fail;

        if ((s = vs_thaw(fp)) == NULL)
            goto fail;

        vh_store(h, key, s);
        v_free(key);

        if ((token = v_thaw_token(fp)) == '}') {
            break;
        } else if (token != ',') {
            v_thaw_expected("comma or close-brace");
            goto fail;
        }
    }

    v_thaw_finish();
    return h;

  fail:
    v_thaw_finish();
    v_destroy(h);
    return NULL;
}

/* Traverse a hash */
int
vh_traverse(vhash *h, int (*func)(void *ptr))
{
    vscalar *s;
    vtable *hp;
    int i, val;
    void *ptr;

    VH_CHECK(h);

    if ((val = func(h)) != 0)
        return val;

    if (v_traverse_seen(h))
        return 0;

    v_push_traverse(h);

    for (i = 0; i < h->tablesize; i++) {
        for (hp = h->table[i]; hp != NULL; hp = hp->next) {
            s = hp->val;
            if (vs_type(s) != V_TYPE_POINTER)
                continue;

            ptr = vs_pget(s);
            if ((val = v_traverse(ptr, func)) == 0)
                continue;

            v_pop_traverse();
            return val;
        }
    }

    v_pop_traverse();

    return 0;
}

/*!
  @brief   Undefine a hash entry.
  @ingroup hash_modify
  @param   h Hash.
  @param   key Entry to undefine.

  If the hash entry doesn't exist, it is created.  The scalar value is then
  set to V_UNDEF.
*/
void
vh_undef(vhash *h, char *key)
{
    unsigned hashval;
    vtable *hp;

    VH_CHECK(h);
    VH_HASH(h, key, hashval);
    VH_FIND(h, key, hashval, hp);

    if (hp != NULL && hp->val != NULL)
        vs_undef(hp->val);
    else
        vh_store(h, key, vs_create(V_TYPE_UNDEF));
}

/*!
  @brief   Return a list of values of a hash (in unspecified order).
  @ingroup hash_convert
  @param   h Hash.
  @return  List of values.
*/
vlist *
vh_values(vhash *h)
{
    vtable *hp;
    vlist *l;

    VH_CHECK(h);

    l = vl_create();
    VH_ITERATE(h, hp, vl_push(l, vs_copy(hp->val)));

    return l;
}

/* Write hash to a stream */
int
vh_write(vhash *h, FILE *fp)
{
    int *info, i;
    vtable *hp;

    VH_CHECK(h);

    /* Write no. of hash entries */
    info = vh_hashinfo(h);
    if (!v_write_long(info[2], fp))
        return 0;

    for (i = 0; i < h->tablesize; i++) {
        for (hp = h->table[i]; hp != NULL; hp = hp->next) {
            /* Write hash key */
            if (!v_write_string(hp->key, fp))
                return 0;

            /* Write hash value */
            if (!vs_write(hp->val, fp))
                return 0;
        }
    }

    return 1;
}

/* Dump XML contents of a hash */
static int
vh_xmldump(vhash *h, FILE *fp)
{
    vtable **entries;
    vscalar *val;
    char *key;
    int i;

    VH_CHECK(h);

    v_xmldump_start(fp);
    entries = vh_entries(h);

    for (i = 0; entries[i] != NULL; i++) {
        key = entries[i]->key;
        val = entries[i]->val;

        if (vs_defined(val)) {
            v_xmldump_tag_start(fp, "entry", "key", key, NULL);

            if (vs_xmldump(val, fp))
                v_xmldump_tag_finish(fp, "entry");
            else
                return 0;
        } else {
            v_xmldump_tag(fp, "entry", "key", key, NULL);
        }
    }

    V_DEALLOC(entries);
    v_xmldump_finish(fp);

    return 1;
}

/* Dump contents of a hash in YAML format */
static int
vh_yamldump(vhash *h, FILE *fp)
{
    int i;

    VH_CHECK(h);

    if (!v_yaml_start(fp))
        return 0;

    if (!v_yaml_write_hash(h, fp))
        return 0;

    if (!v_yaml_finish(fp))
        return 0;

    return 1;
}
