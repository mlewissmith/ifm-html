/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup list Lists
  @ingroup types

  A list is an ordered sequence of scalars.
*/

/*!
  @defgroup list_create Creating and destroying lists
  @ingroup list
*/

/*!
  @defgroup list_access Accessing list elements
  @ingroup list
*/

/*!
  @defgroup list_modify Modifying list elements
  @ingroup list
*/

/*!
  @defgroup list_convert Converting between lists and strings
  @ingroup list
*/

/*!
  @defgroup list_cutpaste Cutting and pasting lists
  @ingroup list
*/

/*!
  @defgroup list_function Applying functions to lists
  @ingroup list
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "vars-config.h"
#include "vars-buffer.h"
#include "vars-list.h"
#include "vars-macros.h"
#include "vars-memory.h"
#include "vars-yaml.h"

#ifndef LIST_DEFAULT_SIZE
#define LIST_DEFAULT_SIZE 16
#endif

#define SENTENCE_END ".!?"

#define LLEN(l)      ((l)->end - (l)->beg + 1)
#define LVAL(l, n)   ((l)->list[(l)->beg + n])
#define LLAST(l)     ((l)->list[++(l)->end])

#define LIST_FIND(l, s, compare, match, retval) do {            \
    int min = 0, max = LLEN(l), val, oldval = 0, cmp;           \
    vscalar *sval;                                              \
    if (compare == NULL) compare = vs_cmp;                      \
    while (1) {                                                 \
        if ((val = (min + max) / 2) == oldval) break;           \
        oldval = val;                                           \
        sval = LVAL(l, val);                                    \
        cmp = compare(&s, &sval);                               \
        if (cmp > 0) min = val;                                 \
        else if (cmp < 0) max = val;                            \
        else break;                                             \
    }                                                           \
    retval = val;                                               \
    if (match) {                                                \
        sval = LVAL(l, val);                                    \
        if (compare(&s, &sval) != 0) retval = V_CMP_NOTFOUND;   \
    } else if (LLEN(l) == 0) {                                  \
        retval = V_CMP_NOTFOUND;                                \
    } else if (compare(&s, &l->list[l->beg]) < 0) {             \
        retval = V_CMP_SMALLER;                                 \
    } else if (compare(&s, &l->list[l->end]) > 0) {             \
        retval = V_CMP_GREATER;                                 \
    }                                                           \
} while (0)

/* Type definition */
struct v_list {
    struct v_header id;         /* Type marker */
    vscalar **list;             /* List elements */
    int initsize;               /* Initial size */
    int cursize;                /* Current size */
    int beg, end;               /* End markers */
};

/* Type variable */
vtype *vlist_type = NULL;

/* Internal functions */
static void vl_need_push(vlist *l, int num);
static void vl_need_unshift(vlist *l, int num);
static int vl_xmldump(vlist *l, FILE *fp);
static int vl_yamldump(vlist *l, FILE *fp);

/*!
  @brief   Append a list to another list.
  @ingroup list_cutpaste
  @param   l List.
  @param   a List to append.
*/
void
vl_append(vlist *l, vlist *a)
{
    int i;

    VL_CHECK(l);
    VL_CHECK(a);

    vl_need_push(l, LLEN(a));
    for (i = a->beg; i <= a->end; i++)
        vl_push(l, vs_copy(a->list[i]));
}

/*!
  @brief   Apply a function to each list element.
  @ingroup list_function
  @param   l List.
  @param   applyfunc Function to apply to each element.
*/
void
vl_apply(vlist *l, void (*applyfunc)(vscalar *s))
{
    int i;

    VL_CHECK(l);

    for (i = l->beg; i <= l->end; i++)
	(*applyfunc)(l->list[i]);
}

/* Check if pointer is a list */
int
vl_check(void *ptr)
{
    return (ptr != NULL && v_type(ptr) == vlist_type);
}

/*!
  @brief   Concatenate two lists and return the result.
  @ingroup list_cutpaste
  @param   a First list.
  @param   b Second list.
  @return  Concatenated list.
*/
vlist *
vl_concat(vlist *a, vlist *b)
{
    vlist *l;
    int i;

    VL_CHECK(a);
    VL_CHECK(b);

    l = vl_create();
    vl_need_push(l, LLEN(a) + LLEN(b));

    for (i = a->beg; i <= a->end; i++)
        vl_push(l, vs_copy(a->list[i]));

    for (i = b->beg; i <= b->end; i++)
        vl_push(l, vs_copy(b->list[i]));

    return l;
}

/*!
  @brief   Return a copy of a list.
  @ingroup list_create
  @param   l List.
  @return  Copy.
*/
vlist *
vl_copy(vlist *l)
{
    vlist *lc;
    int i;

    VL_CHECK(l);
    lc = vl_create();
    vl_need_push(lc, LLEN(l));

    for (i = l->beg; i <= l->end; i++)
	vl_push(lc, vs_copy(l->list[i]));

    return lc;
}

/*!
  @brief   Return a newly-allocated list.
  @ingroup list_create
  @return  New list.
*/
vlist *
vl_create(void)
{
    return vl_create_size(0);
}

/*!
  @brief   Return a newly-allocated list with a given size.
  @ingroup list_create
  @param   size Initial size.
  @return  New list.
*/
vlist *
vl_create_size(unsigned size)
{
    static vheader *id = NULL;
    vlist *l;

    if (size == 0)
        size = LIST_DEFAULT_SIZE;

    if (id == NULL) {
        vl_declare();
        id = v_header(vlist_type);
    }

    l = V_ALLOC(vlist, 1);

    l->id = *id;
    l->cursize = l->initsize = size;
    l->list = V_ALLOC(vscalar *, l->cursize);
    l->beg = 0;
    l->end = -1;

    return l;
}

/* Declare list type */
vtype *
vl_declare(void)
{
    if (vlist_type == NULL) {
        vlist_type = v_create("LIST", "L");
	v_create_func(vlist_type, (void *(*)()) vl_create);
        v_copy_func(vlist_type, (void *(*)()) vl_copy);
        v_next_func(vlist_type, vl_next);
        v_read_func(vlist_type, (void *(*)()) vl_read);
        v_write_func(vlist_type, vl_write);
        v_freeze_func(vlist_type, vl_freeze);
        v_thaw_func(vlist_type, (void *(*)()) vl_thaw);
        v_print_func(vlist_type, vl_print);
        v_destroy_func(vlist_type, vl_destroy);
        v_traverse_func(vlist_type, vl_traverse);
        v_xmldump_func(vlist_type, vl_xmldump);
        v_yamldump_func(vlist_type, vl_yamldump);
    }

    return vlist_type;
}

/*!
  @brief   Return whether a list element is defined.
  @ingroup list_access
  @param   l List.
  @param   num List index.
  @return  Yes or no.
*/
int
vl_defined(vlist *l, int num)
{
    VL_CHECK(l);

    if (num >= 0 && num < LLEN(l))
        return vs_defined(vl_get(l, num));

    return 0;
}

/*!
  @brief   Deallocate a list and its contents.
  @ingroup list_create
  @param   l List.
*/
void
vl_destroy(vlist *l)
{
    VL_CHECK(l);

    vl_empty(l);
    V_DEALLOC(l->list);
    V_DEALLOC(l);
}

/*!
  @brief   Empty a list.
  @ingroup list_modify
  @param   l List.
*/
void
vl_empty(vlist *l)
{
    int i;

    VL_CHECK(l);

    for (i = l->beg; i <= l->end; i++)
        vs_destroy(l->list[i]);

    l->list = V_REALLOC(l->list, vscalar *, l->initsize);
    l->cursize = l->initsize;
    l->beg = 0;
    l->end = -1;
}

/*!
  @brief   Fill text and return a list of text lines.
  @ingroup list_convert
  @param   text Text to fill.
  @param   width Maximum line width, in characters.
  @return  List of strings.

  Treat @c text as human-readable text and fill it into lines no longer
  than the specified width.  The text is split into words delimited by
  whitespace and filled as words separated by a single space.  Words ending
  in one of @c .!? are assumed to end a sentence, and are followed by two
  consecutive spaces.
*/
vlist *
vl_filltext(char *text, int width)
{
    static vbuffer *b = NULL;
    char *word, *line, last;
    int i, len, spaces;
    vlist *words;
    vlist *lines;
    viter iter;

    /* Initialise */
    vb_init(b);
    lines = vl_create();

    /* Split text into words */
    words = vl_split(text, NULL);

    /* Build filled text lines */
    v_iterate(words, iter) {
        word = vl_iter_svalref(iter);
        line = vb_get(b);
        len = strlen(line);

        /* Check for word wrap */
        if (len > 0) {
            last = line[len - 1];
            spaces = (strchr(SENTENCE_END, last) == NULL ? 1 : 2);
            if (len + spaces + (int) strlen(word) > width) {
                vl_spush(lines, line);
                vb_empty(b);
            } else {
                for (i = 0; i < spaces; i++)
                    vb_putc(b, ' ');
            }
        }

        /* Add word to line */
        vb_puts(b, word);
    }

    if (vb_length(b) > 0)
        vl_spush(lines, vb_get(b));

    vl_destroy(words);

    return lines;
}

/* Freeze contents of a list */
int
vl_freeze(vlist *l, FILE *fp)
{
    int i;

    VL_CHECK(l);

    v_freeze_start(fp);

    fputs("{\n", fp);
    v_push_indent();

    for (i = l->beg; i <= l->end; i++) {
        v_indent(fp);
        if (vs_freeze(l->list[i], fp))
            fputs(",\n", fp);
        else
            return 0;
    }

    v_pop_indent();
    v_indent(fp);
    fputc('}', fp);

    v_freeze_finish(fp);
    return 1;
}

/*!
  @brief   Return an indexed element of a list.
  @ingroup list_access
  @param   l List.
  @param   num List index.
  @return  List element.
*/
vscalar *
vl_get(vlist *l, int num)
{
    VL_CHECK(l);

    if (num >= 0 && num < LLEN(l))
        return LVAL(l, num);

    return NULL;
}

/*!
  @brief   Return the head of a list.
  @ingroup list_access
  @param   l List.
  @return  First element in the list.
  @retval  NULL if the list is empty.
*/
vscalar *
vl_head(vlist *l)
{
    VL_CHECK(l);

    return (LLEN(l) > 0 ? l->list[l->beg] : NULL);
}

/*!
  @brief   Insert a scalar into a list.
  @ingroup list_modify
  @param   l List.
  @param   num List index to insert at.
  @param   s Scalar to insert.
  @return  Index of the new entry (-1 means nothing was done).

  If insertion point is past the end of the list, a vl_push() is done
  instead.
*/
int
vl_insert(vlist *l, int num, vscalar *s)
{
    int i;

    VL_CHECK(l);
    VS_CHECK(s);

    if (num >= LLEN(l))
        return vl_push(l, s) - 1;

    if (num < 0) {
        v_exception("vl_insert(): invalid list index");
        return -1;
    }

    vl_need_push(l, 1);
    l->end++;

    for (i = LLEN(l) - 1; i >= num; i--)
        LVAL(l, i + 1) = LVAL(l, i);

    LVAL(l, num) = s;

    return num;
}

/*!
  @brief   Insert a scalar into a list, keeping it sorted.
  @ingroup list_modify
  @param   l List.
  @param   s Scalar to insert.
  @param   compare Comparison function.
  @return  Index of the new entry.

  This function assumes the list is already sorted according to the
  comparison function.  There are no guarantees otherwise.
*/
int
vl_insort(vlist *l, int (*compare)(vscalar **s1, vscalar **s2), vscalar *s)
{
    int num;

    VL_CHECK(l);
    VS_CHECK(s);

    LIST_FIND(l, s, compare, 0, num);

    switch (num) {
    case V_CMP_SMALLER:
        vl_unshift(l, s);
        num = 0;
        break;
    case V_CMP_GREATER:
    case V_CMP_NOTFOUND:
        num = vl_push(l, s) - 1;
        break;
    default:
        num = vl_insert(l, num + 1, s);
        break;
    }

    return num;
}

/*!
  @brief   Join a list into a single string and return it.
  @ingroup list_convert
  @param   l List.
  @param   delim String to insert between elements.
  @return  String (pointer to internal buffer).
*/
char *
vl_join(vlist *l, char *delim)
{
    static char buf[BUFSIZ];
    V_BUF_DECL;
    int i;

    VL_CHECK(l);

    /* Initialise */
    V_BUF_INIT;

    /* Build string */
    for (i = l->beg; i <= l->end; i++) {
        if (i != l->beg)
            V_BUF_ADD(delim);

        V_BUF_ADD(vs_sget_buf(l->list[i], buf));
    }

    return V_BUF_VAL;
}

/*!
  @brief   Join a list of words into a single string and return it.
  @ingroup list_convert
  @param   l List.
  @param   endings Sentence ending characters (or @c NULL).
  @return  String (pointer to internal buffer).

  Like vl_join(), but the delimiter is a single space, or two spaces if a
  word ends in one of the ending characters.  If @c endings is @c NULL, it
  defaults to the standard sentence endings: @c .!?
*/
char *
vl_join_text(vlist *l, char *endings)
{
    static char buf[BUFSIZ];
    V_BUF_DECL;
    char *text;
    int i, len;

    VL_CHECK(l);

    /* Initialise */
    V_BUF_INIT;

    if (endings == NULL)
        endings = SENTENCE_END;

    /* Build string */
    for (i = l->beg; i <= l->end; i++) {
        if (i != l->beg) {
            text = V_BUF_VAL;
            len = strlen(text);
            if (strchr(endings, text[len - 1]) != NULL)
                V_BUF_ADD("  ");
            else
                V_BUF_ADD(" ");
        }

        V_BUF_ADD(vs_sget_buf(l->list[i], buf));
    }

    return V_BUF_VAL;
}

/*!
  @brief   Return length of a list.
  @ingroup list_access
  @param   l List.
  @return  No. of elements.
*/
int
vl_length(vlist *l)
{
    VL_CHECK(l);
    return LLEN(l);
}

/*!
  @brief   Build a list from a list of arguments.
  @ingroup list_create
  @return  New list.

  Each list entry is specified by two parameters: an int type, and a value
  depending on its type. The type can be one of V_TYPE_INT, V_TYPE_FLOAT,
  V_TYPE_DOUBLE, V_TYPE_STRING, V_TYPE_POINTER or V_TYPE_UNDEF. The type of
  the following value should correspond to this, except in the case of
  V_TYPE_UNDEF, where the following value should be omitted. The list of
  values should be terminated by V_TYPE_NULL. If a value has the wrong
  type, or no terminator is supplied, chaos will surely result.
*/
vlist *
vl_list(int type, ...)
{
    va_list ap;
    vlist *l;

    l = vl_create();
    va_start(ap, type);

    while (type != V_TYPE_NULL) {
        switch (type) {
        case V_TYPE_INT:
            vl_ipush(l, va_arg(ap, int));
            break;
        case V_TYPE_FLOAT:
            /* Read as double */
            vl_fpush(l, va_arg(ap, double));
            break;
        case V_TYPE_DOUBLE:
            vl_dpush(l, va_arg(ap, double));
            break;
        case V_TYPE_STRING:
            vl_spush(l, va_arg(ap, char *));
            break;
        case V_TYPE_POINTER:
            vl_ppush(l, va_arg(ap, void *));
            break;
        case V_TYPE_UNDEF:
            vl_push(l, vs_create(V_TYPE_UNDEF));
            break;
        default:
            v_fatal("vl_list(): invalid scalar type in arg list");
        }

        type = va_arg(ap, int);
    }

    va_end(ap);
    return l;
}

/*!
  @brief   Return a list built from a NULL-terminated list of strings.
  @ingroup list_create
  @param   list List of strings, followed by @c NULL.
  @return  New list.
*/
vlist *
vl_makelist(char **list)
{
    vlist *l;

    l = vl_create();
    if (list == NULL)
        return l;

    while (*list != NULL)
	vl_spush(l, *list++);

    return l;
}

/*!
  @brief   Return a list mapped by a function.
  @ingroup list_function
  @param   l List.
  @param   mapfunc Function to map each element with.
  @return  Mapped list.
*/
vlist *
vl_map(vlist *l, vscalar *(*mapfunc)(vscalar *s))
{
    vlist *m;
    int i;

    VL_CHECK(l);

    m = vl_create();
    for (i = l->beg; i <= l->end; i++)
	vl_push(m, (*mapfunc)(l->list[i]));

    return m;
}

/*!
  @brief   Return a sublist of a list satisfying a match function.
  @ingroup list_function
  @param   l List.
  @param   matchfunc Function to test each element with.
  @return  List of matched elements.
*/
vlist *
vl_match(vlist *l, int (*matchfunc)(vscalar *s))
{
    vlist *m;
    int i;

    VL_CHECK(l);

    m = vl_create();
    for (i = l->beg; i <= l->end; i++)
	if ((*matchfunc)(l->list[i]))
	    vl_push(m, vs_copy(l->list[i]));

    return m;
}

/* Increase list size to the right if required */
static void
vl_need_push(vlist *l, int num)
{
    int offset;

    for (offset = 0;
         l->cursize + offset <= l->end + num;
         offset += l->initsize);

    if (offset > 0) {
        l->cursize += offset;
        l->list = V_REALLOC(l->list, vscalar *, l->cursize);
    }
}

/* Increase list size to the left if required */
static void
vl_need_unshift(vlist *l, int num)
{
    int offset, i;

    for (offset = 0;
         offset <= num - l->beg;
         offset += l->initsize);

    if (offset > 0) {
        l->cursize += offset;
        l->list = V_REALLOC(l->list, vscalar *, l->cursize);

        for (i = l->end; i >= l->beg; i--)
            l->list[offset + i] = l->list[i];

        l->beg += offset;
        l->end += offset;
    }
}

/*!
  @brief   Iterate over a list.
  @ingroup list_access
  @param   iter Iterator.
  @return  Whether to continue.
  @see     v_iter(), v_next(), v_iterate()

  Iterate over the values in a list.  Returns whether there are any more
  entries.  The iterator must have been initialized via v_iter() with a
  list object.

  After this call, the iterator structure will contain data on the next
  list entry, which you can access using vl_iter_val().
*/
int
vl_next(viter *iter)
{
    vlist *l = iter->object;

    VL_CHECK(l);

    if (iter->ipos < LLEN(l)) {
        iter->pval[0] = LVAL(l, iter->ipos);
        iter->ipos++;
        return 1;
    }

    return 0;
}

/*!
  @brief   Pop and return the last element of a list.
  @ingroup list_modify
  @param   l List.
  @return  Last element.
  @retval  NULL if the list is empty.
*/
vscalar *
vl_pop(vlist *l)
{
    VL_CHECK(l);

    if (LLEN(l) == 0)
        return NULL;

    return l->list[l->end--];
}

/* Print contents of a list */
void
vl_print(vlist *l, FILE *fp)
{
    int i;

    VL_CHECK(l);

    v_print_start();
    v_push_indent();

    v_print_type(vlist_type, l, fp);

    for (i = l->beg; i <= l->end; i++) {
        v_indent(fp);
        fprintf(fp, "%d  ", i - l->beg);
        v_print(l->list[i], fp);
    }

    v_pop_indent();
    v_print_finish();
}

/*!
  @brief   Push an element onto end of a list.
  @ingroup list_modify
  @param   l List.
  @param   s Scalar to add.
  @return  Length of the list.
*/
int
vl_push(vlist *l, vscalar *s)
{
    VL_CHECK(l);
    VS_CHECK(s);

    vl_need_push(l, 1);
    LLAST(l) = s;

    return LLEN(l);
}

/* Read list from a stream */
vlist *
vl_read(FILE *fp)
{
    vscalar *s;
    int len, i;
    vlist *l;

    l = vl_create();

    /* Length of list */
    if (!v_read_long(&len, fp))
        return NULL;

    /* List elements */
    for (i = 0; i < len; i++) {
        if ((s = vs_read(fp)) != NULL)
            vl_push(l, s);
        else
            return NULL;
    }

    return l;
}

/*!
  @brief   Remove a scalar from a list.
  @ingroup list_modify
  @param   l List.
  @param   num List index to remove.
  @return  Removed scalar (or @c NULL if not found).
*/
vscalar *
vl_remove(vlist *l, int num)
{
    vscalar *s;
    int i;

    VL_CHECK(l);

    if (num < 0) {
        v_exception("vl_remove(): negative list index");
        return NULL;
    } else if (num >= LLEN(l)) {
        return NULL;
    }

    s = LVAL(l, num);

    for (i = num; i < LLEN(l); i++)
        LVAL(l, i) = LVAL(l, i + 1);
    l->end--;

    return s;
}

/*!
  @brief   Return a reversed list.
  @ingroup list_convert
  @param   l List.
  @return  Reversed list.
*/
vlist *
vl_reverse(vlist *l)
{
    vlist *r;
    int i;

    VL_CHECK(l);

    r = vl_create();

    for (i = l->end; i >= l->beg; i--)
	vl_push(r, vs_copy(l->list[i]));

    return r;
}

/*!
  @brief   Do binary search on a sorted list.
  @ingroup list_function
  @param   l List.
  @param   s Scalar to search for.
  @param   compare Comparison function.
  @param   match Exact-match flag.
  @param   destroy Whether to destroy scalar key.
  @return  List index.
  @see     vl_search() and associated macros.

  Perform a binary search on a sorted list to find a given scalar key.  The
  supplied comparison function should be the same as the one used to sort
  the list (see vl_sort()).  If the function is NULL, then vs_cmp() is
  used. Returns the list index of the matching scalar, if it is found in
  the list.

  If the key is not found in the list, the return value is as follows. If
  the @c match flag is set, or the list is empty, then V_CMP_NOTFOUND is
  returned.  If the key is smaller than the first element of the list, then
  V_CMP_SMALLER is returned.  If the key is greater than the last element,
  then V_CMP_GREATER is returned.  Otherwise the index of the largest list
  element not greater than the key is returned.

  If @c destroy is set, the scalar key is destroyed.  This is a gross hack
  to get the search macros to behave nicely.
*/
int
vl_search_list(vlist *l, vscalar *s,
               int (*compare)(vscalar **s1, vscalar **s2),
               int match, int destroy)
{
    int retval;

    VL_CHECK(l);
    VS_CHECK(s);

    LIST_FIND(l, s, compare, match, retval);

    /*
     * Dispose of scalar if required (this is a gross hack to get
     * the type-dependent macros to work nicely).
     */
    if (destroy)
        vs_destroy(s);

    return retval;
}

/*!
  @brief   Shift an element from the front of a list.
  @ingroup list_modify
  @param   l List.
  @return  Scalar removed.
  @retval  NULL if the list is empty.
*/
vscalar *
vl_shift(vlist *l)
{
    VL_CHECK(l);

    if (LLEN(l) > 0)
        return l->list[l->beg++];

    return NULL;
}

/*!
  @brief   Sort a list.
  @ingroup list_function
  @param   l List.
  @param   compare Comparison function.
  @return  The original list.

  Sort the elements of a list in-place.  If @c compare is @c NULL, vs_cmp()
  is used instead.
*/
void
vl_sort(vlist *l, int (*compare)(vscalar **s1, vscalar **s2))
{
    VL_CHECK(l);

    if (compare == NULL)
	compare = vs_cmp;

    if (LLEN(l) > 1)
        qsort(l->list, (size_t) LLEN(l), sizeof(vscalar *),
              (int (*)(const void *, const void *)) compare);
}

/*!
  @brief   Splice another list into a list and return the removed part.
  @ingroup list_cutpaste
  @param   l List.
  @param   offset Place to start splicing from.
  @param   length No. of elements to remove.
  @param   r List to replace with.
  @return  Removed list.

  Remove the elements indicated by @c offset and @c length from a list, and
  replace them with the elements of list @c r, if not @c NULL.  The list
  grows or shrinks as required.  If @c length is zero, removes everything
  from @c offset onward.  A negative @c offset means count from the end of
  the list instead of the beginning.
*/
vlist *
vl_splice(vlist *l, int offset, int length, vlist *r)
{
    int i, count, diff;
    vlist *s;

    VL_CHECK(l);
    if (r != NULL)
        VL_CHECK(r);

    if (offset < 0)
        offset += LLEN(l);
    if (offset < 0)
        offset = 0;

    if (length <= 0)
        length = LLEN(l) - offset;

    s = vl_create();

    /* Do nothing if list isn't affected */
    if (offset >= LLEN(l))
        return s;

    /* Create spliced list */
    vl_need_push(s, length);
    for (i = offset; i < offset + length; i++)
        if (i < LLEN(l))
            LLAST(s) = LVAL(l, i);

    /* Extend list if required */
    count = offset + length - LLEN(l);
    if (r != NULL && LLEN(r) > length)
        count += LLEN(r) - length;
    if (count > 0) {
        vl_need_push(l, count);
        for (i = 0; i < count; i++)
            LLAST(l) = vs_create(V_TYPE_UNDEF);
    }

    /* Shift entries to make room for replacement */
    diff = (r == NULL ? 0 : LLEN(r)) - length;

    if (diff > 0) {
        for (i = l->end; i >= offset + length; i--)
            LVAL(l, i + diff) = LVAL(l, i);
    } else {
        for (i = offset + length; i <= l->end; i++)
            LVAL(l, i + diff) = LVAL(l, i);
    }

    l->end += diff;

    /* Do replacement if required */
    if (r != NULL)
        for (i = 0; i < LLEN(r); i++)
            LVAL(l, offset + i) = vs_copy(LVAL(r, i));

    return s;
}

/*!
  @brief   General split function.
  @ingroup list_convert
  @param   string String to split.
  @param   sep Field separator characters.
  @param   quotes Quote character(s).
  @param   nullfields Whether to allow empty fields.
  @return  List of elements.
  @see     vl_split(), vl_qsplit(), vl_nsplit(), vl_qnsplit()
*/
vlist *
vl_split_string(char *string, char *sep, char *quotes, int nullfields)
{
    char openquote, closequote, *str = string, *cp;
    int addword = 0;
    vscalar *val;
    vlist *l;

    /* Initialise */
    l = vl_create();

    if (string == NULL)
	return l;

    if (sep == NULL)
        sep = " \t\r\n\f";

    /* Get quote characters if required */
    if (quotes != NULL) {
	switch (strlen(quotes)) {
	case 1:
	    openquote = closequote = quotes[0];
	    break;
	case 2:
	    openquote = quotes[0];
	    closequote = quotes[1];
	    break;
	default:
	    v_exception("vl_split_string(): invalid quote argument");
            return l;
	}
    }

    /* Scan string */
    while (*str != '\0') {
	if (strchr(sep, *str) != NULL) {
	    /* Separator -- add null field if required */
            if (nullfields)
                vl_spush(l, "");
	    str++;
	} else if (quotes != NULL && *str == openquote) {
	    /* Quote character -- find closing quote */
	    str++;
	    cp = str;
	    while (*cp != closequote && *cp != '\0')
		cp++;
	    addword = 1;
	} else {
	    /* Word character -- find end of word */
	    cp = str;
	    while (strchr(sep, *cp) == NULL && *cp != '\0')
		cp++;
	    addword = 1;
	}

	/* Add word to list if found */
	if (addword) {
            val = vs_sstore_len(NULL, str, (size_t) (cp - str));
            vl_push(l, val);

	    str = cp;
            if (quotes != NULL && *str == closequote)
                str++;
            if (*str != '\0')
                str++;
	    addword = 0;
	}
    }

    return l;
}

/*!
  @brief   Set an indexed element of a list.
  @ingroup list_modify
  @param   l List.
  @param   num Index to store.
  @param   s Scalar to store there.
*/
void
vl_store(vlist *l, int num, vscalar *s)
{
    int i, count;

    VL_CHECK(l);
    VS_CHECK(s);

    if (num < 0) {
        v_exception("vl_store(): attempt to reference element %d of a list",
                    num);
        return;
    }

    if ((count = num - LLEN(l) + 1) > 0) {
        vl_need_push(l, count);
        for (i = 0; i < count; i++)
            LLAST(l) = vs_create(V_TYPE_UNDEF);
    }

    if (LVAL(l, num) != s && LVAL(l, num) != NULL)
        vs_destroy(LVAL(l, num));

    LVAL(l, num) = s;
}

/*!
  @brief   Return the tail of a list.
  @ingroup list_access
  @param   l List.
  @return  Last element of the list.
  @retval  NULL if the list is empty.
*/
vscalar *
vl_tail(vlist *l)
{
    VL_CHECK(l);

    return (LLEN(l) > 0 ? l->list[l->end] : NULL);
}

/* Thaw a list from file */
vlist *
vl_thaw(FILE *fp)
{
    vscalar *s;
    int token;
    vlist *l;

    v_thaw_start();

    l = vl_create();

    if (!v_thaw_follow(fp, '{', "open-brace"))
        goto fail;

    while (1) {
        if (v_thaw_peek(fp) == '}') {
            v_thaw_token(fp);
            break;
        }

        if ((s = vs_thaw(fp)) == NULL)
            goto fail;

        vl_push(l, s);

        if ((token = v_thaw_token(fp)) == '}') {
            break;
        } else if (token != ',') {
            v_thaw_expected("comma or close-brace");
            goto fail;
        }
    }

    v_thaw_finish();
    return l;

  fail:
    v_thaw_finish();
    v_destroy(l);
    return NULL;
}

/* Traverse a list */
int
vl_traverse(vlist *l, int (*func)(void *ptr))
{
    int i, val;

    VL_CHECK(l);

    if ((val = func(l)) != 0)
        return val;

    if (v_traverse_seen(l))
        return 0;

    v_push_traverse(l);

    for (i = l->beg; i <= l->end; i++) {
        if (vs_type(l->list[i]) == V_TYPE_POINTER &&
            (val = v_traverse(vs_pget(l->list[i]), func)) != 0) {
            v_pop_traverse();
            return val;
        }
    }

    v_pop_traverse();

    return 0;
}

/*!
  @brief   Undefine a list element.
  @ingroup list_modify
  @param   l List.
  @param   num Index to undefine.
*/
void
vl_undef(vlist *l, int num)
{
    VL_CHECK(l);

    if (num >= 0 && num < LLEN(l))
        vs_undef(vl_get(l, num));
}

/*!
  @brief   Unshift a new element onto the front of a list.
  @ingroup list_modify
  @param   l List.
  @param   s Scalar to add.
*/
void
vl_unshift(vlist *l, vscalar *s)
{
    VL_CHECK(l);
    VS_CHECK(s);

    vl_need_unshift(l, 1);
    l->list[--l->beg] = s;
}

/* Write list to a stream */
int
vl_write(vlist *l, FILE *fp)
{
    int i, len;

    VL_CHECK(l);

    /* Length of list */
    len = LLEN(l);
    if (!v_write_long(len, fp))
        return 0;

    /* List elements */
    for (i = l->beg; i <= l->end; i++)
        if (!vs_write(l->list[i], fp))
            return 0;

    return 1;
}

/* Dump contents of a list in XML format */
static int
vl_xmldump(vlist *l, FILE *fp)
{
    int i;

    VL_CHECK(l);

    v_xmldump_start(fp);

    for (i = l->beg; i <= l->end; i++) {
        v_xmldump_tag_start(fp, "entry", NULL);

        if (!vs_xmldump(l->list[i], fp))
            return 0;

        v_xmldump_tag_finish(fp, "entry");
    }

    v_xmldump_finish(fp);
    return 1;
}

/* Dump contents of a list in YAML format */
static int
vl_yamldump(vlist *l, FILE *fp)
{
    int i;

    VL_CHECK(l);

    if (!v_yaml_start(fp))
        return 0;

    if (!v_yaml_write_list(l, fp))
        return 0;

    if (!v_yaml_finish(fp))
        return 0;

    return 1;
}
