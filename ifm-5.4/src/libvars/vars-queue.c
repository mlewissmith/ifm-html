/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup queue Priority queues
  @ingroup types

  A priority queue is an object into which items can be inserted at various
  different priorities.  The items then get removed in order of their
  priority.  They're useful (for example) in simulation systems, where the
  priority is a measure of 'event time', so that events occur in the
  correct time order.  Each entry in a priority queue is a scalar that has
  a priority attached.
*/

/*!
  @defgroup queue_create Creating and destroying queues
  @ingroup queue
*/

/*!
  @defgroup queue_access Accessing queue entries
  @ingroup queue
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vars-config.h"
#include "vars-macros.h"
#include "vars-memory.h"
#include "vars-queue.h"

#ifndef QUEUE_DEFAULT_SIZE
#define QUEUE_DEFAULT_SIZE 16
#endif

#define QLEN(q)                 ((q)->entries - 1)
#define QVAL(q, n)              ((q)->list[n])

struct v_elt {
    vscalar *val;               /* Value */
    double priority;            /* Priority */
};

struct v_queue {
    struct v_header id;         /* Type marker */
    struct v_elt **list;        /* List of entries */
    int queuesize;              /* Initial size */
    int size;                   /* Current size */
    int entries;                /* No. of entries */
};

/* Internal abbreviations */
typedef struct v_elt velt;

/* Type variable */
vtype *vqueue_type = NULL;

/* Internal functions */
static void vq_downheap(vqueue *q, int n);
static velt *vq_new(vscalar *s, double priority);
static void vq_require(vqueue *q, int num);
static void vq_upheap(vqueue *q, int n);
static int vq_xmldump(vqueue *q, FILE *fp);

/* Check if pointer is a queue */
int
vq_check(void *ptr)
{
    return (ptr != NULL && v_type(ptr) == vqueue_type);
}

/*!
  @brief   Return a copy of a queue.
  @ingroup queue_create
  @param   q Queue.
  @return  Copy.
*/
vqueue *
vq_copy(vqueue *q)
{
    vqueue *copy;
    int i;

    VQ_CHECK(q);

    copy = vq_create();
    vq_require(copy, q->entries);

    for (i = 1; i < q->entries; i++)
        vq_store(copy, vs_copy(QVAL(q, i)->val), QVAL(q, i)->priority);

    return copy;
}

/*!
  @brief   Create a new priority queue.
  @ingroup queue_create
  @return  New queue.
*/
vqueue *
vq_create(void)
{
    return vq_create_size(QUEUE_DEFAULT_SIZE);
}

/*!
  @brief   Create a new priority queue with a given initial size.
  @ingroup queue_create
  @param   size Initial size.
  @return  New queue.
*/
vqueue *
vq_create_size(int size)
{
    static vheader *id = NULL;
    vqueue *q;

    size = V_MAX(size, 10);

    if (id == NULL) {
        vq_declare();
        id = v_header(vqueue_type);
    }

    q = V_ALLOC(vqueue, 1);
    q->id = *id;

    q->size = q->queuesize = size;
    q->list = V_ALLOC(velt *, q->size);
    QVAL(q, 0) = vq_new(NULL, 0.0);
    q->entries = 1;

    return q;
}

/* Declare queue type */
vtype *
vq_declare(void)
{
    if (vqueue_type == NULL) {
        vqueue_type = v_create("QUEUE", "Q");
	v_create_func(vqueue_type, (void *(*)()) vq_create);
        v_copy_func(vqueue_type, (void *(*)()) vq_copy);
        v_next_func(vqueue_type, vq_next);
        v_read_func(vqueue_type, (void *(*)()) vq_read);
        v_write_func(vqueue_type, vq_write);
        v_freeze_func(vqueue_type, vq_freeze);
        v_thaw_func(vqueue_type, (void *(*)()) vq_thaw);
        v_print_func(vqueue_type, vq_print);
        v_destroy_func(vqueue_type, vq_destroy);
        v_traverse_func(vqueue_type, vq_traverse);
        v_xmldump_func(vqueue_type, vq_xmldump);
    }

    return vqueue_type;
}

/*!
  @brief   Deallocate a queue.
  @ingroup queue_create
  @param   q Queue.
*/
void
vq_destroy(vqueue *q)
{
    VQ_CHECK(q);

    vq_empty(q);
    V_DEALLOC(QVAL(q, 0));
    V_DEALLOC(q->list);
    V_DEALLOC(q);
}

/* Rearrange heap downwards */
static void
vq_downheap(vqueue *q, int n)
{
    int num;
    velt *e;

    e = QVAL(q, n);

    while (n <= QLEN(q) / 2) {
        num = n * 2;
        if (num < QLEN(q))
            if (QVAL(q, num)->priority < QVAL(q, num + 1)->priority)
                num++;

        if (e->priority > QVAL(q, num)->priority)
            break;

        QVAL(q, n) = QVAL(q, num);
        n = num;
    }

    QVAL(q, n) = e;
}

/*!
  @brief   Empty a queue.
  @ingroup queue_access
  @param   q Queue.
*/
void
vq_empty(vqueue *q)
{
    int i;

    VQ_CHECK(q);

    for (i = 1; i < q->entries; i++) {
        if (QVAL(q, i)->val != NULL)
            vs_destroy(QVAL(q, i)->val);
        V_DEALLOC(QVAL(q, i));
    }

    q->entries = 1;
}

/* Freeze contents of a queue */
int
vq_freeze(vqueue *q, FILE *fp)
{
    velt *e;
    int i;

    VQ_CHECK(q);

    v_freeze_start(fp);

    fputs("{\n", fp);
    v_push_indent();

    for (i = 1; i < q->entries; i++) {
        e = QVAL(q, i);

        v_indent(fp);
        fprintf(fp, "%g", e->priority);

        if (vs_defined(e->val)) {
            fputs(" = ", fp);
            if (vs_freeze(e->val, fp))
                fputs(",\n", fp);
            else
                return 0;
        }
    }

    v_pop_indent();
    v_indent(fp);
    fputc('}', fp);

    v_freeze_finish(fp);

    return 1;
}

/*!
  @brief   Remove and return the value with next priority.
  @ingroup queue_access
  @param   q Queue.
  @return  Next value.
*/
vscalar *
vq_get(vqueue *q)
{
    vscalar *s;
    velt *e;

    VQ_CHECK(q);

    if (q->entries == 1)
        return NULL;

    /* Remove next value */
    e = QVAL(q, 1);
    s = e->val;
    V_DEALLOC(e);

    QVAL(q, 1) = QVAL(q, --q->entries);

    /* Reorder entries */
    vq_downheap(q, 1);

    return s;
}

/*!
  @brief   Return the value with highest priority.
  @ingroup queue_access
  @param   q Queue.
  @return  Next value.
*/
vscalar *
vq_head(vqueue *q)
{
    VQ_CHECK(q);
    return (QLEN(q) > 0 ? QVAL(q, 1)->val : NULL);
}

/*!
  @brief   Return no. of entries in the queue.
  @ingroup queue_access
  @param   q Queue.
  @return  No. of entries.
*/
int
vq_length(vqueue *q)
{
    VQ_CHECK(q);
    return QLEN(q);
}

/* Create a new queue entry */
static velt *
vq_new(vscalar *s, double priority)
{
    velt *e;

    e = V_ALLOC(velt, 1);

    e->val = s;
    e->priority = priority;

    return e;
}

/*!
  @brief   Iterate over a queue.
  @ingroup queue_access
  @param   iter Iterator.
  @return  Whether to continue.
  @see     v_iter(), v_next(), v_iterate()

  Iterate over the values in a queue, in an apparently random order.
  Returns whether there are any more entries.  The iterator must have been
  initialized via v_iter() with a list object.

  After this call, the iterator structure will contain data on the next
  queue entry, which you can access using vq_iter_priority() and
  vq_iter_val().
*/
int
vq_next(viter *iter)
{
    vqueue *q = iter->object;
    velt *elt;

    VQ_CHECK(q);

    if (iter->ipos < QLEN(q)) {
        elt = QVAL(q, iter->ipos + 1);
        iter->dval[0] = elt->priority;
        iter->pval[0] = elt->val;
        iter->ipos++;
        return 1;
    }

    return 0;
}

/* Print contents of a queue */
void
vq_print(vqueue *q, FILE *fp)
{
    int i;

    VQ_CHECK(q);

    v_print_start();
    v_push_indent();

    v_print_type(vqueue_type, q, fp);

    for (i = 1; i < q->entries; i++) {
        v_indent(fp);
        fprintf(fp, "PRIORITY %g => ", QVAL(q, i)->priority);
        v_print(QVAL(q, i)->val, fp);
    }

    v_pop_indent();
    v_print_finish();
}

/* Read queue from a stream */
vqueue *
vq_read(FILE *fp)
{
    double priority;
    vscalar *s;
    int num, i;
    vqueue *q;

    q = vq_create();

    /* No. of entries */
    if (!v_read_long(&num, fp))
        return NULL;

    vq_require(q, num);

    /* The entries */
    for (i = 0; i < num; i++) {
        if (!v_read_double(&priority, fp))
            return NULL;

        if ((s = vs_read(fp)) == NULL)
            return NULL;

        vq_store(q, s, priority);
    }

    return q;
}

/* Increase queue size if required */
static void
vq_require(vqueue *q, int num)
{
    int extra = 0;

    while (q->size + extra <= num)
        extra += q->queuesize;

    if (extra > 0) {
        q->size += extra;
        q->list = V_REALLOC(q->list, velt *, q->size);
    }
}

/*!
  @brief   Add an entry to a queue.
  @ingroup queue_access
  @param   q Queue.
  @param   s Entry to add.
  @param   priority Priority of the added entry.
*/
void
vq_store(vqueue *q, vscalar *s, double priority)
{
    VQ_CHECK(q);
    VS_CHECK(s);

    /* Add the entry */
    vq_require(q, q->entries + 1);
    QVAL(q, q->entries++) = vq_new(s, priority);

    /* Make sure sentinel is always biggest */
    if (QVAL(q, 0)->priority < priority + 1)
        QVAL(q, 0)->priority = priority + 1;

    /* Reorder the queue */
    vq_upheap(q, q->entries - 1);
}

/* Thaw a queue from file */
vqueue *
vq_thaw(FILE *fp)
{
    double priority;
    vscalar *val;
    vqueue *q;
    int token;

    v_thaw_start();

    q = vq_create();

    if (!v_thaw_follow(fp, '{', "open-brace"))
        goto fail;

    while (1) {
        if (v_thaw_peek(fp) == '}') {
            v_thaw_token(fp);
            break;
        }

        if (!v_thaw_double(fp, &priority))
            goto fail;

        if (v_thaw_peek(fp) == '=') {
            v_thaw_token(fp);
            if ((val = vs_thaw(fp)) == NULL)
                goto fail;
        } else {
            val = vs_create(V_TYPE_UNDEF);
        }

        vq_store(q, val, priority);

        if ((token = v_thaw_token(fp)) == '}') {
            break;
        } else if (token != ',') {
            v_thaw_expected("comma or close-brace");
            goto fail;
        }
    }

    v_thaw_finish();

    return q;

  fail:
    vq_destroy(q);
    return NULL;
}

/* Traverse a queue */
int
vq_traverse(vqueue *q, int (*func)(void *ptr))
{
    int i, val;

    VQ_CHECK(q);

    if ((val = func(q)) != 0)
        return val;

    if (v_traverse_seen(q))
        return 0;

    v_push_traverse(q);

    for (i = 1; i < q->entries; i++) {
        if (vs_type(QVAL(q, i)->val) == V_TYPE_POINTER &&
            (val = v_traverse(vs_pget(QVAL(q, i)->val), func)) != 0) {
            v_pop_traverse();
            return val;
        }
    }

    v_pop_traverse();

    return 0;
}

/* Rearrange heap upwards */
static void
vq_upheap(vqueue *q, int n)
{
    velt *e;

    e = QVAL(q, n);

    while (n > 0 && QVAL(q, n / 2)->priority < e->priority) {
         QVAL(q, n) = QVAL(q, n / 2);
         n /= 2;
    }

    QVAL(q, n) = e;
}

/* Write queue to a stream */
int
vq_write(vqueue *q, FILE *fp)
{
    int i, len;

    VQ_CHECK(q);

    /* No. of entries */
    len = QLEN(q);
    if (!v_write_long(len, fp))
        return 0;

    /* The entries */
    for (i = 1; i < q->entries; i++) {
        if (!v_write_double(QVAL(q, i)->priority, fp))
            return 0;

        if (!vs_write(QVAL(q, i)->val, fp))
            return 0;
    }

    return 1;
}

/* Dump contents of a queue in XML format */
static int
vq_xmldump(vqueue *q, FILE *fp)
{
    char buf[20];
    velt *e;
    int i;

    VQ_CHECK(q);

    v_xmldump_start(fp);

    for (i = 1; i < q->entries; i++) {
        e = QVAL(q, i);

        sprintf(buf, "%g", e->priority);

        if (vs_defined(e->val)) {
            v_xmldump_tag_start(fp, "entry", "priority", buf, NULL);

            if (vs_xmldump(e->val, fp))
                v_xmldump_tag_finish(fp, "entry");
            else
                return 0;
        } else {
            v_xmldump_tag(fp, "entry", "priority", buf, NULL);
        }
    }

    v_xmldump_finish(fp);

    return 1;
}
