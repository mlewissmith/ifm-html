/*
 * This file is part of IFM (Interactive Fiction Mapper), copyright (C)
 * Glenn Hutchings 1997-2008.
 *
 * IFM comes with ABSOLUTELY NO WARRANTY.  This is free software, and you
 * are welcome to redistribute it under certain conditions; see the file
 * COPYING for details.
 */

/*!
  @defgroup graph Graphs
  @ingroup types

  A graph consists of a set of nodes, some of which are connected by links.
  Each link may have a given size, and may be in one direction only, or
  both ways.  With a graph you can ask questions like, 'What is the
  shortest path between two nodes?', or 'What nodes can I reach from a
  given node?'.

  Each node is named by a string, and each link is specified by the two
  nodes it connects.  Both nodes and links may also have a scalar value
  attached.
*/

/*!
  @defgroup graph_create Creating and destroying graphs
  @ingroup graph
*/

/*!
  @defgroup graph_modify Modifying graph attributes
  @ingroup graph
*/

/*!
  @defgroup graph_access Retrieving graph information
  @ingroup graph
*/

/*!
  @defgroup graph_connect Getting connection information
  @ingroup graph

  These functions return various sorts of connection information about a
  graph.

  A graph can have several user-defined connection functions, which are
  used when nodes are being visited.  These functions can control which
  nodes and links are allowed to be used, and also the size of links.  In
  this way, you can dynamically change the characteristics of graphs
  without having to actually change graph connections.

  If you are calculating paths from a given node to many others, you can
  gain a measure of efficiency by 'caching' path information.  This
  involves calculating the paths from the given node to all other reachable
  nodes in a single pass, and using that information in subsequent path
  function calls.
*/

/*!
  @defgroup graph_sort Topological sorting
  @ingroup graph

  A topological sorting of a graph is a list of all its nodes in such an
  order that if node A links to node B, then node A appears earlier in the
  list than node B.  There may be none, one or many distinct orderings of
  nodes which satisfy this condition.  No ordering is possible if there is
  a 'cycle' in the nodes -- that is, if it is possible to start from a
  node, follow one or more links and arrive back at the start node.
  Clearly, a minimum condition for a valid sorting is that each link in the
  graph must be one-way.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vars-config.h"
#include "vars-graph.h"
#include "vars-hash.h"
#include "vars-macros.h"
#include "vars-memory.h"
#include "vars-queue.h"

/* Link a parent with a child */
#define LINK(parent, head, tail, child, prev, next)                     \
        if (parent->head == NULL) parent->head = child;                 \
        child->prev = parent->tail;                                     \
        if (parent->tail != NULL) parent->tail->next = child;           \
        parent->tail = child

/* Unlink a child from its parent */
#define UNLINK(parent, head, tail, child, prev, next)                   \
        if (child->prev == NULL)                                        \
                parent->head = child->next;                             \
        else                                                            \
                child->prev->next = child->next;                        \
        if (child->next == NULL)                                        \
                parent->tail = child->prev;                             \
        else                                                            \
                child->next->prev = child->prev

#define FINDNODE(g, node, n)                                            \
        n = vh_pget((g)->nodemap, node)

#define FINDLINK(g, n1, n2, l)                                          \
        sprintf(findbuf, "%d|%d", (n1)->id, (n2)->id);                  \
        l = vh_pget((g)->linkmap, findbuf)

#define USENODE(g, n, dist)                                             \
        ((g)->use_node == NULL || (*g->use_node)((n)->name,             \
                                                 (n)->val,              \
                                                 dist))

#define USELINK(g, l)                                                   \
        ((g)->use_link == NULL || (*g->use_link)((l)->from->name,       \
                                                 (l)->to->name,         \
                                                 (l)->val))

#define INIT_VISIT                                                      \
        searchflag++; pathcount++; if (cache_flag) cachecount++

#define VISITED(n)                                                      \
        ((n)->visit == searchflag)

#define VISIT(n)                                                        \
        (n)->visit = searchflag;                                        \
        if (cache_flag)                                                 \
                (n)->cachevisit = cachecount

#define SEEN(n)                                                         \
        ((n)->seen == searchflag)

#define LOOKAT(n)                                                       \
        (n)->seen = searchflag

#define INIT_PATH                                                       \
        pathcount++

#define SETPATH(n, l)                                                   \
        (n)->path = l;                                                  \
        (n)->pathflag = pathcount;                                      \
        if (cache_flag)                                                 \
                (n)->cache = l, (n)->cacheflag = cachecount

#define HASPATH(n)                                                      \
        (cache_flag                                                     \
                ? ((n)->cacheflag == cachecount &&                      \
                   (n)->cache != NULL &&                                \
                   (n)->cachevisit == cachecount)                       \
                : ((n)->pathflag == pathcount &&                        \
                   (n)->path != NULL))

#define SETDIST(l, d)                                                   \
        (l)->dist = d; if (cache_flag) (l)->cachedist = d

#define CHECK_CACHE(g, n)                                               \
        (cache_flag = ((g)->use_cache && (g)->cache == n))

#define NOCACHE                                                         \
        cache_flag = 0

#define NOPATH -999

/* Type definition */
struct v_graph {
    struct v_header id;         /* Type marker */

    /* Quick look-up tables */
    struct v_hash *nodemap;     /* Node name -> node mapping */
    struct v_hash *linkmap;     /* Link name -> link mapping */

    /* Counts of various things */
    unsigned int nodes;         /* No. of nodes */
    unsigned int links;         /* No. of links */
    int idcount;                /* Node ID counter */

    /* Node/link pointers */
    struct v_node *nhead;       /* Head node in list */
    struct v_node *ntail;       /* Tail node in list */
    struct v_link *lhead;       /* Head link in list */
    struct v_link *ltail;       /* Tail link in list */

    /* Iteration pointers */
    struct v_node *npos;        /* Node iteration pointer */
    struct v_link *lpos;        /* Link iteration pointer */

    /* Search functions */
    int (*use_node)(char *node, vscalar *s, double dist);
    int (*use_link)(char *node1, char *node2, vscalar *s);
    double (*link_size)(char *node1, char *node2, vscalar *s);

    /* Cache information */
    struct v_node *cache;       /* Cache node */
    int use_cache;              /* Whether to use cache */
};

struct v_node {
    char *name;                 /* Node name */
    struct v_scalar *val;       /* Node value */
    int id;                     /* Node ID */
    int pnum;                   /* Print number */

    /* Links */
    struct v_node *nprev;       /* Previous node in graph list */
    struct v_node *nnext;       /* Next node in graph list */
    struct v_link *fhead;       /* Head link in 'from' list */
    struct v_link *ftail;       /* Tail link in 'from' list */
    struct v_link *thead;       /* Head link in 'to' list */
    struct v_link *ttail;       /* Tail link in 'to' list */

    /* Search parameters */
    int visit;                  /* Visit flag */
    int seen;                   /* Seen flag */

    int pathflag;               /* Path flag */
    struct v_link *path;        /* Previous link in path */

    int cacheflag;              /* Cache flag */
    int cachevisit;             /* Cache visit flag */
    struct v_link *cache;       /* Previous link in cached path */

    /* Usage parameters */
    int used;                   /* Whether used by at least one link */
};

struct v_link {
    struct v_scalar *val;       /* Link value */
    struct v_node *from;        /* Node linked from */
    struct v_node *to;          /* Node linked to */

    /* Links */
    struct v_link *lprev;       /* Previous link in graph list */
    struct v_link *lnext;       /* Next link in graph list */
    struct v_link *fprev;       /* Previous link in node 'from' list */
    struct v_link *fnext;       /* Next link in node 'from' list */
    struct v_link *tprev;       /* Previous link in node 'to' list */
    struct v_link *tnext;       /* Next link in node 'to' list */

    /* Search parameters */
    double dist;                /* Distance from start node */
    double cachedist;           /* Cached distance */

    /* Usage parameters */
    int oneway;                 /* Whether link is one-way */
    int opposite;               /* Whether link has an opposite */
};

/* Internal type abbreviations */
typedef struct v_node vnode;
typedef struct v_link vlink;

/* Search types */
enum {
    V_DEPTH, V_BREADTH, V_PRIORITY
};

/* Type variable */
vtype *vgraph_type = NULL;

/* Global search flags */
static int searchflag = 0;
static int pathcount = 0;

/* Cache parameters */
static int cachecount = 0;
static int cache_flag = 0;
static int caching_now = 0;

/* Topological sort data */
static vlist *tsort_cycles = NULL;

/* Find-link buffer */
static char findbuf[20];

/* Internal functions */
static vlist *vg_build_path(vgraph *g, vnode *n);
static void vg_delete_link(vgraph *g, vlink *l);
static void vg_delete_node(vgraph *g, vnode *n);
static vlink *vg_getlink(vgraph *g, char *node1, char *node2);
static vnode *vg_getnode(vgraph *g, char *node);
static void vg_getusage(vgraph *g);
static double vg_link_size(vgraph *g, vlink *l);
static vlink *vg_newlink(vgraph *g, vnode *n1, vnode *n2, char *map);
static vnode *vg_newnode(vgraph *g, char *node);
static void vg_tsort_visit(vgraph *g, vnode *n, vlist *order);
static vnode *vg_visit(vgraph *g, vnode *from, vnode *to, int type,
                       vlist *visit);
static int vg_xmldump(vgraph *g, FILE *fp);

/* Build a path given a start node */
static vlist *
vg_build_path(vgraph *g, vnode *n)
{
    vnode *start = n;
    vlist *path;

    path = vl_create();
    vl_sunshift(path, n->name);

    while (HASPATH(n)) {
        if (cache_flag)
            n = n->cache->from;
        else
            n = n->path->from;

        if (n == start)
            break;

        vl_sunshift(path, n->name);
    }

    return path;
}

/*!
  @brief   Return whether currently updating path cache.
  @ingroup graph_connect
  @return  Yes or no.

  This function is for use by user-defined connection functions.
*/
int
vg_caching(void)
{
    return caching_now;
}

/* Check if pointer is a graph */
int
vg_check(void *ptr)
{
    return (ptr != NULL && v_type(ptr) == vgraph_type);
}

/*!
  @brief   Return list of connected groups of nodes.
  @ingroup graph_connect
  @param   g Graph.
  @return  List of groups.

  Return all the connected components of the graph.  A connected component
  is a list of nodes, each of which can be reached from the others.  A path
  does not exist between nodes on two different connected components.  The
  return value is a list, each element of which is in turn a list of node
  names (a connected component).  Note that connected components are only
  properly defined for graphs in which each link is a two-way connection
  (i.e. if node A links to node B, then node B also links to node A).
*/
vlist *
vg_connected(vgraph *g)
{
    vlist *list, *visit;
    vnode *n;

    VG_CHECK(g);

    /* Initialise */
    list = vl_create();
    if (g->nodes == 0)
        return list;

    NOCACHE;
    INIT_VISIT;

    /* Visit all nodes */
    for (n = g->nhead; n != NULL; n = n->nnext) {
        /* Skip if visited */
        if (VISITED(n))
            continue;

        /* Create new visit list */
        visit = vl_create();
        vl_ppush(list, visit);

        /* Visit nodes */
        vg_visit(g, n, NULL, V_BREADTH, visit);
    }

    return list;
}

/*!
  @brief   Return a copy of a graph.
  @ingroup graph_create
  @param   g Graph.
  @return  Copy.
*/
vgraph *
vg_copy(vgraph *g)
{
    vgraph *copy;
    vnode *n;
    vlink *l;

    VG_CHECK(g);

    copy = vg_create();

    for (n = g->nhead; n != NULL; n = n->nnext)
        vg_node_store(copy, n->name, vs_copy(n->val));

    for (l = g->lhead; l != NULL; l = l->lnext)
        vg_link_oneway_store(g, l->from->name, l->to->name, vs_copy(l->val));

    return copy;
}

/*!
  @brief   Return a newly-created graph.
  @ingroup graph_create
  @return  New graph.
*/
vgraph *
vg_create(void)
{
    static vheader *id = NULL;
    vgraph *g;

    if (id == NULL) {
        vg_declare();
        id = v_header(vgraph_type);
    }

    g = V_ALLOC(vgraph, 1);
    g->id = *id;

    g->nodemap = vh_create();
    g->linkmap = vh_create();

    g->nodes = g->links = 0;

    g->nhead = g->ntail = NULL;
    g->lhead = g->ltail = NULL;
    g->npos = NULL;
    g->lpos = NULL;
    g->idcount = 0;

    g->use_node = NULL;
    g->use_link = NULL;
    g->link_size = NULL;

    g->cache = NULL;
    g->use_cache = 0;

    return g;
}

/* Declare graph type */
vtype *
vg_declare(void)
{
    if (vgraph_type == NULL) {
        vgraph_type = v_create("GRAPH", "G");
	v_create_func(vgraph_type, (void *(*)()) vg_create);
        v_copy_func(vgraph_type, (void *(*)()) vg_copy);
        v_read_func(vgraph_type, (void *(*)()) vg_read);
        v_write_func(vgraph_type, vg_write);
        v_freeze_func(vgraph_type, vg_freeze);
        v_thaw_func(vgraph_type, (void *(*)()) vg_thaw);
        v_print_func(vgraph_type, vg_print);
        v_destroy_func(vgraph_type, vg_destroy);
        v_traverse_func(vgraph_type, vg_traverse);
        v_xmldump_func(vgraph_type, vg_xmldump);
    }

    return vgraph_type;
}

/*!
  @brief   Delete a node given its name.
  @ingroup graph_modify
  @param   g Graph.
  @param   node Node to delete.

  Firstly, delete all the links involving this node.  Then delete the node.
*/
void
vg_delete(vgraph *g, char *node)
{
    vnode *n;

    VG_CHECK(g);

    FINDNODE(g, node, n);
    if (n != NULL)
        vg_delete_node(g, n);
}

/* Delete a link */
static void
vg_delete_link(vgraph *g, vlink *l)
{
    /* Unlink it */
    UNLINK(g, lhead, ltail, l, lprev, lnext);
    UNLINK(l->from, thead, ttail, l, tprev, tnext);
    UNLINK(l->to, fhead, ftail, l, fprev, fnext);

    /* Destroy its value (if any) */
    if (l->val != NULL)
        vs_destroy(l->val);

    /* Destroy it */
    g->links--;
    V_DEALLOC(l);
}

/* Delete a node */
static void
vg_delete_node(vgraph *g, vnode *n)
{
    vlink *l, *lnext;

    /* Delete all connected links */
    for (l = n->fhead; l != NULL; l = lnext) {
        lnext = l->fnext;
        vg_delete_link(g, l);
    }

    for (l = n->thead; l != NULL; l = lnext) {
        lnext = l->tnext;
        vg_delete_link(g, l);
    }

    /* Unlink it */
    UNLINK(g, nhead, ntail, n, nprev, nnext);

    /* Destroy its value (if any) */
    if (n->val != NULL)
        vs_destroy(n->val);

    /* Destroy it */
    g->nodes--;
    V_DEALLOC(n->name);
    V_DEALLOC(n);
}

/*!
  @brief   Deallocate a graph.
  @ingroup graph_create
  @param   g Graph.
*/
void
vg_destroy(vgraph *g)
{
    vnode *n, *nnext;

    VG_CHECK(g);

    /* Delete nodes */
    for (n = g->nhead; n != NULL; n = nnext) {
        nnext = n->nnext;
        vg_delete_node(g, n);
    }

    /* Destroy node/link maps */
    vh_destroy(g->nodemap);
    vh_destroy(g->linkmap);

    /* Destroy it */
    V_DEALLOC(g);
}

/* Freeze contents of a graph */
int
vg_freeze(vgraph *g, FILE *fp)
{
    int first = 1;
    vlink *l;
    vnode *n;

    VG_CHECK(g);

    /* Get usage information */
    vg_getusage(g);

    /* Start freezing */
    v_freeze_start(fp);

    fputs("{\n", fp);
    v_push_indent();

    /* Freeze nodes */
    for (n = g->nhead; n != NULL; n = n->nnext) {
        if (n->used && !vs_defined(n->val))
            continue;

        if (first)
            first = 0;
        else
            fputs(",\n", fp);

        v_indent(fp);
        fputs("NODE ", fp);

        if (!v_freeze_string(n->name, fp))
            return 0;

        if (vs_defined(n->val)) {
            fputs(" = ", fp);
            if (!vs_freeze(n->val, fp))
                return 0;
        }
    }

    /* Freeze links */
    for (l = g->lhead; l != NULL; l = l->lnext) {
        if (l->opposite)
            continue;

        if (first)
            first = 0;
        else
            fputs(",\n", fp);

        v_indent(fp);
        fputs("LINK ", fp);
        if (!v_freeze_string(l->from->name, fp))
            return 0;

        fputs(" TO ", fp);
        if (!v_freeze_string(l->to->name, fp))
            return 0;

        if (l->oneway)
            fputs(" ONEWAY", fp);

        if (vs_defined(l->val)) {
            fputs(" = ", fp);
            if (!vs_freeze(l->val, fp))
                return 0;
        }
    }

    fputc('\n', fp);
    v_pop_indent();
    v_indent(fp);
    fputc('}', fp);

    v_freeze_finish(fp);

    return 1;
}

/* Get a link given its two nodes, or create a new one */
static vlink *
vg_getlink(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2;
    vlink *l;

    n1 = vg_getnode(g, node1);
    n2 = vg_getnode(g, node2);

    FINDLINK(g, n1, n2, l);
    if (l == NULL)
        l = vg_newlink(g, n1, n2, findbuf);

    return l;
}

/* Get a node given its name, or create a new one */
static vnode *
vg_getnode(vgraph *g, char *node)
{
    vnode *n;

    FINDNODE(g, node, n);
    if (n == NULL)
        n = vg_newnode(g, node);

    return n;
}

/* Get usage information for graph's nodes and links */
static void
vg_getusage(vgraph *g)
{
    vlink *l, *lback;
    vnode *n;

    for (n = g->nhead; n != NULL; n = n->nnext)
        n->used = 0;

    for (l = g->lhead; l != NULL; l = l->lnext) {
        l->from->used = 1;
        l->to->used = 1;
        l->oneway = 1;
        l->opposite = 0;
    }

    for (l = g->lhead; l != NULL; l = l->lnext) {
        if (l->opposite)
            continue;

        FINDLINK(g, l->to, l->from, lback);
        if (lback != NULL && vs_equal(l->val, lback->val)) {
            l->oneway = 0;
            lback->opposite = 1;
        }
    }
}

/*!
  @brief   Return number of links in a graph.
  @ingroup graph_access
  @param   g Graph.
  @return  No. of links.
*/
int
vg_link_count(vgraph *g)
{
    VG_CHECK(g);
    return g->links;
}

/*!
  @brief   Return whether a link exists.
  @ingroup graph_access
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
  @return  Yes or no.
*/
int
vg_link_exists(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2;
    vlink *l;

    VG_CHECK(g);

    FINDNODE(g, node1, n1);
    FINDNODE(g, node2, n2);
    if (n1 == NULL || n2 == NULL)
        return 0;

    FINDLINK(g, n1, n2, l);

    return (l != NULL);
}

/*!
  @brief   Return data associated with a link.
  @ingroup graph_access
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
  @return  Value.
  @retval  NULL if the link doesn't exist.
*/
vscalar *
vg_link_get(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2;
    vlink *l;

    VG_CHECK(g);

    FINDNODE(g, node1, n1);
    FINDNODE(g, node2, n2);
    if (n1 == NULL || n2 == NULL)
        return NULL;

    FINDLINK(g, n1, n2, l);

    return (l == NULL ? NULL : l->val);
}

/*!
  @brief   Declare a one-way link and associate data with it.
  @ingroup graph_modify
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
  @param   s Value to store (or @c NULL).

  Firstly, if any of the named nodes don't exist, create them (with
  undefined scalar values).  Then, if a link between the nodes doesn't
  exist, create it.  Finally, replace its current scalar value (if any)
  with the given value.
*/
void
vg_link_oneway_store(vgraph *g, char *node1, char *node2, vscalar *s)
{
    vlink *l;

    VG_CHECK(g);

    l = vg_getlink(g, node1, node2);
    if (l->val != NULL)
        vs_destroy(l->val);
    l->val = s;
}

/* Return the size of a link */
static double
vg_link_size(vgraph *g, vlink *l)
{
    double size;

    if (g->link_size != NULL) {
        size = (*g->link_size)(l->from->name, l->to->name, l->val);
    } else {
        switch (vs_type(l->val)) {
        case V_TYPE_INT:
        case V_TYPE_FLOAT:
        case V_TYPE_DOUBLE:
            size = vs_dget(l->val);
            break;
        default:
            size = 1.0;
            break;
        }
    }

    return (size >= 0.0 ? size : 0.0);
}

/*!
  @brief   Set the link-size function of a graph.
  @ingroup graph_connect
  @param   g Graph.
  @param   func Function to calculate link size.
*/
void
vg_link_size_function(vgraph *g, double (*func)(char *node1, char *node2,
                                                vscalar *s))
{
    VG_CHECK(g);
    g->link_size = func;
}

/*!
  @brief   Declare a two-way link and associate the same data with both.
  @ingroup graph_modify
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
  @param   s Value to set.
*/
void
vg_link_store(vgraph *g, char *node1, char *node2, vscalar *s)
{
    vlink *l;

    VG_CHECK(g);

    l = vg_getlink(g, node1, node2);
    if (l->val != NULL)
        vs_destroy(l->val);
    l->val = s;

    l = vg_getlink(g, node2, node1);
    if (l->val != NULL)
        vs_destroy(l->val);
    l->val = vs_copy(s);
}

/* Create a new link */
static vlink *
vg_newlink(vgraph *g, vnode *n1, vnode *n2, char *map)
{
    vlink *l;

    l = V_ALLOC(vlink, 1);

    l->val = vs_create(V_TYPE_UNDEF);
    l->from = n1;
    l->to = n2;

    l->lprev = l->lnext = NULL;
    l->fprev = l->fnext = NULL;
    l->tprev = l->tnext = NULL;

    LINK(g, lhead, ltail, l, lprev, lnext);
    LINK(n1, thead, ttail, l, tprev, tnext);
    LINK(n2, fhead, ftail, l, fprev, fnext);

    vh_pstore(g->linkmap, map, l);
    g->links++;

    return l;
}

/* Create a new node */
static vnode *
vg_newnode(vgraph *g, char *node)
{
    vnode *n;

    n = V_ALLOC(vnode, 1);

    n->name = V_STRDUP(node);
    n->id = ++g->idcount;
    n->val = vs_create(V_TYPE_UNDEF);

    n->nprev = n->nnext = NULL;
    n->fhead = n->ftail = NULL;
    n->thead = n->ttail = NULL;

    n->visit = n->seen = n->pathflag = n->cacheflag = -1;
    n->path = n->cache = NULL;
    n->cachevisit = -1;

    LINK(g, nhead, ntail, n, nprev, nnext);

    vh_pstore(g->nodemap, node, n);
    g->nodes++;

    return n;
}

/*!
  @brief   Return number of nodes in a graph.
  @ingroup graph_access
  @param   g Graph.
  @return  No. of nodes.
*/
int
vg_node_count(vgraph *g)
{
    VG_CHECK(g);
    return g->nodes;
}

/*!
  @brief   Return whether a node exists.
  @ingroup graph_access
  @param   g Graph.
  @param   node Node name.
  @return  Yes or no.
*/
int
vg_node_exists(vgraph *g, char *node)
{
    VG_CHECK(g);

    return vh_exists(g->nodemap, node);
}

/*!
  @brief   Return a list of nodes linked to a given node.
  @ingroup graph_access
  @param   g Graph.
  @param   node Node name.
  @return  List of nodes.
  @retval  @c NULL if the node doesn't exist.
*/
vlist *
vg_node_from(vgraph *g, char *node)
{
    vlist *list;
    vnode *n;
    vlink *l;

    VG_CHECK(g);
    FINDNODE(g, node, n);
    if (n == NULL)
        return NULL;

    list = vl_create();
    for (l = n->fhead; l != NULL; l = l->fnext)
        vl_spush(list, l->from->name);

    return list;
}

/*!
  @brief   Return data associated with a node.
  @ingroup graph_access
  @param   g Graph.
  @param   node Node name.
  @return  Value.
  @retval  @c NULL if the node doesn't exist.
*/
vscalar *
vg_node_get(vgraph *g, char *node)
{
    vnode *n;

    VG_CHECK(g);

    FINDNODE(g, node, n);
    return (n == NULL ? NULL : n->val);
}

/*!
  @brief   Return count of from/to links of a given node.
  @ingroup graph_access
  @param   g Graph.
  @param   node Node name.
  @param[out]   from Pointer to no. of from links, or @c NULL.
  @param[out]   to Pointer to no. of to links, or @c NULL.
  @return  Total no. of from/to links.
*/
int
vg_node_links(vgraph *g, char *node, int *from, int *to)
{
    int links = 0;
    vnode *n;
    vlink *l;

    VG_CHECK(g);

    FINDNODE(g, node, n);
    if (n == NULL)
        return 0;

    if (from != NULL)
        *from = 0;

    for (l = n->fhead; l != NULL; l = l->fnext) {
        links++;
        if (from != NULL)
            *from++;
    }

    if (to != NULL)
        *to = 0;

    for (l = n->thead; l != NULL; l = l->tnext) {
        links++;
        if (to != NULL)
            *to++;
    }

    return links;
}

/*!
  @brief   Return sorted list of nodes in a graph.
  @ingroup graph_access
  @param   g Graph.
  @return  List of nodes.
*/
vlist *
vg_node_list(vgraph *g)
{
    VG_CHECK(g);
    return vh_keys(g->nodemap);
}

/*!
  @brief   Declare a node and associate data with it.
  @ingroup graph_modify
  @param   g Graph.
  @param   node Node name.
  @param   s Value.
*/
void
vg_node_store(vgraph *g, char *node, vscalar *s)
{
    vnode *n;

    VG_CHECK(g);

    n = vg_getnode(g, node);
    if (n->val != NULL)
        vs_destroy(n->val);
    n->val = s;    
}

/*!
  @brief   Return a list of nodes a given node links to.
  @ingroup graph_access
  @param   g Graph.
  @param   node Node name.
  @return  List of nodes.
  @retval  @c NULL if the node doesn't exist.
*/
vlist *
vg_node_to(vgraph *g, char *node)
{
    vlist *list;
    vnode *n;
    vlink *l;

    VG_CHECK(g);

    FINDNODE(g, node, n);
    if (n == NULL)
        return NULL;

    list = vl_create();
    for (l = n->thead; l != NULL; l = l->tnext)
        vl_spush(list, l->to->name);

    return list;
}

/*!
  @brief   Cache paths from a given node.
  @ingroup graph_connect
  @param   g Graph.
  @param   node Node to cache from.
  @return  Distance of furthest reachable node.

  Do a single pass over a graph, starting from @c node, recording path
  information for use in subsequent calls to vg_path_nodes(),
  vg_path_length() or vg_path_exists().  If the first node in these calls
  is the start node specified here, then this path information is used
  instead of traversing the graph.  If @c node is @c NULL, caching of path
  information is turned off (and the return value is not relevant).  Note
  that if cached path information is used, the user-defined connection
  functions will not be called.
*/
double
vg_path_cache(vgraph *g, char *node)
{
    vnode *n;

    VG_CHECK(g);

    if (node != NULL) {
        FINDNODE(g, node, n);
        if (n != NULL) {
            g->cache = n;
            g->use_cache = 1;
            cache_flag = 1;

            INIT_VISIT;
            caching_now = 1;
            n = vg_visit(g, n, NULL, V_PRIORITY, NULL);
            caching_now = 0;

            if (n == NULL || n->path == NULL)
                return 0;

            return n->path->dist;
        }
    }

    g->cache = NULL;
    g->use_cache = 0;

    return NOPATH;
}

/*!
  @brief   Return whether a path exists between two nodes.
  @ingroup graph_connect
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
  @return  Yes or no.

  Like vg_path_nodes(), but just return whether there is a path between the
  given nodes.
*/
int
vg_path_exists(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2;

    VG_CHECK(g);

    FINDNODE(g, node1, n1);
    FINDNODE(g, node2, n2);
    if (n1 == NULL || n2 == NULL)
        return 0;

    if (n1 == n2)
        return 1;

    if (CHECK_CACHE(g, n1))
        return HASPATH(n2);
        
    INIT_VISIT;
    if (vg_visit(g, n1, n2, V_PRIORITY, NULL) == NULL)
        return 0;

    return 1;
}

/*!
  @brief   Return list of info about path twixt two nodes.
  @ingroup graph_connect
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
  @return  List.

  Like vg_path_nodes(), but prepends the length of the path as the first
  element in the list.
*/
vlist *
vg_path_info(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2, *n;
    vlist *path;
    double len;

    VG_CHECK(g);

    FINDNODE(g, node1, n1);
    FINDNODE(g, node2, n2);
    if (n1 == NULL || n2 == NULL)
        return NULL;

    if (n1 == n2) {
        path = vl_create();
        vl_spush(path, node1);
        len = 0;
    } else if (CHECK_CACHE(g, n1)) {
        if (HASPATH(n2))
            path = vg_build_path(g, n2);
        else
            return NULL;
        len = n2->cache->cachedist;
    } else {
        INIT_VISIT;
        if ((n = vg_visit(g, n1, n2, V_PRIORITY, NULL)) == NULL)
            return NULL;
        path = vg_build_path(g, n);
        len = n2->path->dist;
    }

    vl_dunshift(path, len);
    return path;
}

/*!
  @brief   Return length of a path between two nodes.
  @ingroup graph_connect
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
  @return  Path length.
  @retval  Negative if there's no path.
*/
double
vg_path_length(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2, *n;

    VG_CHECK(g);

    FINDNODE(g, node1, n1);
    FINDNODE(g, node2, n2);
    if (n1 == NULL || n2 == NULL)
        return NOPATH;

    if (n1 == n2)
        return 0;

    if (CHECK_CACHE(g, n1))
        return (HASPATH(n2) ? n2->cache->cachedist : NOPATH);

    INIT_VISIT;
    if ((n = vg_visit(g, n1, n2, V_PRIORITY, NULL)) == NULL)
        return NOPATH;

    return n->path->dist;
}

/*!
  @brief   Return the links in a path twixt two nodes.
  @ingroup graph_connect
  @param   g Graph.
  @param   from From node.
  @param   to To node.
  @return  List of data associated with each link in the path.
  @retval  @c NULL if a path doesn't exist.
*/
vlist *
vg_path_links(vgraph *g, char *from, char *to)
{
    vlist *links, *list;
    vscalar *s;
    int i, n;

    VG_CHECK(g);

    if ((list = vg_path_nodes(g, from, to)) == NULL)
        return NULL;

    links = vl_create();
    n = vl_length(list);

    for (i = 0; i < n - 1; i++) {
        from = vl_sgetref(list, i);
        to   = vl_sgetref(list, i + 1);
        s = vg_link_get(g, from, to);
        vl_push(links, vs_copy(s));
    }

    vl_destroy(list);
    return links;
}

/*!
  @brief   Return list of nodes twixt two nodes.
  @ingroup graph_connect
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
  @return  List of nodes.
  @retval  @c NULL if a path doesn't exist.

  Given two nodes, return a list of nodes which constitutes a shortest path
  from one node to the other.  Both the start and finish nodes are included
  in the list.

  The shortest path is calculated using the 'size' of each link.  The size
  of a link is defined as follows.  Firstly, if the graph has an associated
  link size function, then that is called on each link to find the link
  size.  If not, then the link's scalar value is examined.  If this has a
  numeric value (either V_TYPE_INT, V_TYPE_FLOAT or V_TYPE_DOUBLE) then
  that is the link size.  Otherwise, the link size is 1.  A negative link
  size is silently rounded up to zero.
*/
vlist *
vg_path_nodes(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2, *n;
    vlist *path;

    VG_CHECK(g);

    FINDNODE(g, node1, n1);
    FINDNODE(g, node2, n2);
    if (n1 == NULL || n2 == NULL)
        return NULL;

    if (n1 == n2) {
        path = vl_create();
        vl_spush(path, node1);
    } else if (CHECK_CACHE(g, n1)) {
        if (HASPATH(n2))
            path = vg_build_path(g, n2);
        else
            return NULL;
    } else {
        INIT_VISIT;
        if ((n = vg_visit(g, n1, n2, V_PRIORITY, NULL)) == NULL)
            return NULL;
        path = vg_build_path(g, n);
    }

    return path;
}

/*!
  @brief   Return sorted list of nodes reachable from a given node.
  @ingroup graph_connect
  @param   g Graph.
  @param   node Node to examine.
  @return  List of nodes reachable from it.
*/
vlist *
vg_path_reachable(vgraph *g, char *node)
{
    vlist *list;
    char *other;
    viter iter;

    VG_CHECK(g);

    vg_path_cache(g, node);
    list = vl_create();

    v_iterate(g->nodemap, iter) {
        other = vh_iter_key(iter);
        if (vg_path_exists(g, node, other))
            vl_spush(list, other);
    }

    vl_sort(list, NULL);
    return list;
}

/* Print contents of a graph */
void
vg_print(vgraph *g, FILE *fp)
{
    vnode *n;
    vlink *l;

    VG_CHECK(g);

    v_print_start();
    v_push_indent();

    v_print_type(vgraph_type, g, fp);

    for (n = g->nhead; n != NULL; n = n->nnext) {
        v_indent(fp);
        fprintf(fp, "NODE %s => ", n->name);
        v_print(n->val, fp);
    }

    for (l = g->lhead; l != NULL; l = l->lnext) {
        v_indent(fp);
        fprintf(fp, "LINK %s TO %s => ", l->from->name, l->to->name);
        v_print(l->val, fp);
    }

    v_pop_indent();
    v_print_finish();
}

/* Read graph from a stream */
vgraph *
vg_read(FILE *fp)
{
    char *name, *node1, *node2;
    int n1, n2, num;
    vlist *nodes;
    vscalar *s;
    vgraph *g;
    int i;

    g = vg_create();
    nodes = vl_create();

    /* Read nodes */
    if (!v_read_long(&num, fp))
        return NULL;

    for (i = 0; i < num; i++) {
        if ((name = v_read_string(fp)) == NULL)
            return NULL;
        if ((s = vs_read(fp)) == NULL)
            return NULL;

        vg_node_store(g, name, s);
        vl_spush(nodes, name);
    }

    /* Read links */
    if (!v_read_long(&num, fp))
        return NULL;

    for (i = 0; i < num; i++) {
        if (!v_read_long(&n1, fp))
            return NULL;
        if (!v_read_long(&n2, fp))
            return NULL;
        if ((s = vs_read(fp)) == NULL)
            return NULL;

        node1 = vl_sgetref(nodes, n1);
        node2 = vl_sgetref(nodes, n2);
        vg_link_oneway_store(g, node1, node2, s);
    }

    vl_destroy(nodes);

    return g;
}

/* Thaw a graph from file */
vgraph *
vg_thaw(FILE *fp)
{
    int token, oneway;
    char *arg1, *arg2;
    vscalar *val;
    vgraph *g;

    v_thaw_start();

    g = vg_create();

    if (!v_thaw_follow(fp, '{', "open-brace"))
        goto fail;

    while (1) {
        if (v_thaw_peek(fp) == '}') {
            v_thaw_token(fp);
            break;
        }

        if (!v_thaw_follow(fp, V_TOKEN_ID, "NODE or LINK"))
            goto fail;

        if (V_STREQ(v_thaw_svalue, "NODE")) {
            if (!v_thaw_follow(fp, V_TOKEN_STRING, "node name string"))
                goto fail;
            arg1 = V_STRDUP(v_thaw_svalue);

            if (v_thaw_peek(fp) == '=') {
                v_thaw_token(fp);
                val = vs_thaw(fp);
            } else {
                val = vs_create(V_TYPE_UNDEF);
            }

            if (val != NULL) 
                vg_node_store(g, arg1, val);

            v_free(arg1);

            if (val == NULL)
                goto fail;
        } else if (V_STREQ(v_thaw_svalue, "LINK")) {
            if (!v_thaw_follow(fp, V_TOKEN_STRING, "node name string"))
                goto fail;
            arg1 = V_STRDUP(v_thaw_svalue);

            if (!v_thaw_follow(fp, V_TOKEN_ID, "TO")) {
                v_free(arg1);
                goto fail;
            }

            if (!v_thaw_follow(fp, V_TOKEN_STRING, "node name string"))
                goto fail;
            arg2 = V_STRDUP(v_thaw_svalue);

            oneway = 0;
            if (v_thaw_peek(fp) == V_TOKEN_ID) {
                v_thaw_token(fp);
                if (V_STREQ(v_thaw_svalue, "ONEWAY")) {
                    oneway = 1;
                } else {
                    v_thaw_expected("comma, equals or ONEWAY");
                    v_free(arg1);
                    v_free(arg2);
                    goto fail;
                }
            }

            if (v_thaw_peek(fp) == '=') {
                v_thaw_token(fp);
                val = vs_thaw(fp);
            } else {
                val = vs_create(V_TYPE_UNDEF);
            }

            if (val != NULL) {
                if (oneway)
                    vg_link_oneway_store(g, arg1, arg2, val);
                else
                    vg_link_store(g, arg1, arg2, val);
            }

            v_free(arg1);
            v_free(arg2);

            if (val == NULL)
                goto fail;
        } else {
            v_thaw_expected("NODE or LINK");
            goto fail;
        }

        if ((token = v_thaw_token(fp)) == '}') {
            break;
        } else if (token != ',') {
            v_thaw_expected("comma or close-brace");
            goto fail;
        }
    }

    v_thaw_finish();
    return g;

  fail:
    v_thaw_finish();
    v_destroy(g);
    return NULL;
}

/* Traverse a graph */
int
vg_traverse(vgraph *g, int (*func)(void *ptr))
{
    vnode *n;
    vlink *l;
    int val;

    VG_CHECK(g);

    if ((val = func(g)) != 0)
        return val;

    if (v_traverse_seen(g))
        return 0;

    v_push_traverse(g);

    /* Get usage information */
    vg_getusage(g);

    /* Traverse nodes */
    for (n = g->nhead; n != NULL; n = n->nnext)
        if (n->val != NULL && vs_type(n->val) == V_TYPE_POINTER)
            if ((val = v_traverse(vs_pget(n->val), func)) != 0)
                return val;

    /* Traverse links */
    for (l = g->lhead; l != NULL; l = l->lnext) {
        if (l->opposite)
            continue;

        if (l->val != NULL && vs_type(l->val) == V_TYPE_POINTER)
            if ((val = v_traverse(vs_pget(l->val), func)) != 0)
                return val;
    }

    v_pop_traverse();

    return 0;
}

/*!
  @brief   Return topologically sorted list of nodes.
  @ingroup graph_sort
  @param   g Graph.
  @return  List of nodes.
  @retval  NULL if cycles exist.
*/
vlist *
vg_tsort(vgraph *g)
{
    vlist *order, *cycle;
    vnode *n;

    VG_CHECK(g);

    /* Initialise */
    NOCACHE;
    INIT_VISIT;
    order = vl_create();

    if (tsort_cycles == NULL) {
        tsort_cycles = vl_create();
    } else while (vl_length(tsort_cycles) > 0) {
        cycle = vl_ppop(tsort_cycles);
        vl_destroy(cycle);
    }

    /* Visit all nodes */
    for (n = g->nhead; n != NULL; n = n->nnext) {
        if (VISITED(n))
            continue;

        if (V_DEBUG(V_DBG_INTERN))
            fprintf(stderr, "Starting from '%s'", n->name);

        INIT_PATH;
        vg_tsort_visit(g, n, order);
    }

    if (vl_length(tsort_cycles) > 0) {
        vl_destroy(order);
        return NULL;
    }

    return order;
}

/*!
  @brief   Return cycles found in the last topological sort (if any).
  @ingroup graph_sort
  @return  List of cycles.

  If vg_tsort() returned @c NULL, this function returns all the distinct
  cycles found in the graph, as a list of lists of nodes.  Note that this
  function returns a pointer to an internal list, which will be clobbered
  the next time vg_tsort() is called.
*/
vlist *
vg_tsort_cycles(void)
{
    if (tsort_cycles == NULL)
        return NULL;

    if (vl_length(tsort_cycles) == 0)
        return NULL;

    return tsort_cycles;
}

/* Recursive topological sort visit function */
static void
vg_tsort_visit(vgraph *g, vnode *n, vlist *order)
{
    vnode *f, *start, *first;
    vlist *cycle;
    int newcycle;
    vlink *l;

    /* Visit this node */
    if (V_DEBUG(V_DBG_INTERN))
        fprintf(stderr, "   Visiting '%s'", n->name);

    VISIT(n);

    /* Visit all nodes that point to this one */
    for (l = n->fhead; l != NULL; l = l->fnext) {
        f = l->from;

        if (V_DEBUG(V_DBG_INTERN))
            fprintf(stderr, "      Looking at %s -> %s", f->name, n->name);

        if (!VISITED(f)) {
            /* Unvisited -- record path to it */
            SETPATH(n, l);
            vg_tsort_visit(g, f, order);
        } else if (HASPATH(f)) {
            /* Visited before -- check for cycle */
            start = f;
            first = n;
            newcycle = 1;

            while (HASPATH(f)) {
                if (SEEN(f))
                    newcycle = 0;
                f = f->path->from;
                if (f == first)
                    break;
            }

            /* If it's a new cycle, add it to the cycle list */
            if (f == first && newcycle) {
                LOOKAT(f);
                cycle = vg_build_path(g, start);
                vl_ppush(tsort_cycles, cycle);
                if (V_DEBUG(V_DBG_INTERN))
                    fprintf(stderr, "\nCycle found: %s\n",
                            vl_join(cycle, " -> "));
            }
        }
    }

    /* Add current node to sorted list */
    vl_spush(order, n->name);
}

/*!
  @brief   Unlink two nodes.
  @ingroup graph_modify
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.

  Like vg_unlink_oneway(), but delete the link in both directions.
*/
void
vg_unlink(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2;
    vlink *l;

    VG_CHECK(g);

    FINDNODE(g, node1, n1);
    FINDNODE(g, node2, n2);
    if (n1 == NULL || n2 == NULL)
        return;

    FINDLINK(g, n1, n2, l);
    if (l != NULL)
        vg_delete_link(g, l);

    FINDLINK(g, n2, n1, l);
    if (l != NULL)
        vg_delete_link(g, l);
}

/*!
  @brief   Unlink two nodes (one-way only).
  @ingroup graph_modify
  @param   g Graph.
  @param   node1 From node.
  @param   node2 To node.
*/
void
vg_unlink_oneway(vgraph *g, char *node1, char *node2)
{
    vnode *n1, *n2;
    vlink *l;

    VG_CHECK(g);

    FINDNODE(g, node1, n1);
    FINDNODE(g, node2, n2);
    if (n1 == NULL || n2 == NULL)
        return;

    FINDLINK(g, n1, n2, l);
    if (l != NULL)
        vg_delete_link(g, l);
}

/*!
  @brief   Toggle path cache usage.
  @ingroup graph_connect
  @param   g Graph.
  @param   flag Whether to cache paths.
*/
void
vg_use_cache(vgraph *g, int flag)
{
    VG_CHECK(g);
    g->use_cache = flag;
}

/*!
  @brief   Set the use-link function of a graph.
  @ingroup graph_connect
  @param   g Graph.
  @param   func Function to test link usage.

  Like vg_use_node_function(), but for links.
*/
void
vg_use_link_function(vgraph *g, int (*func)(char *node1,
                                            char *node2,
                                            vscalar *s))
{
    VG_CHECK(g);
    g->use_link = func;
}


/*!
  @brief   Set the use-node function of a graph.
  @ingroup graph_connect
  @param   g Graph.
  @param   func Function to test node usage.

  Declare a function which will be used in path-finding functions to
  determine whether a given node should be used in the path.  This function
  is called for each node with its name, scalar value and distance from the
  start node as arguments.  If it returns zero, then that node is excluded
  from the path.  If the function is @c NULL, this removes any
  previously-declared function.
*/
void
vg_use_node_function(vgraph *g, int (*func)(char *node,
                                            vscalar *s,
                                            double dist))
{
    VG_CHECK(g);
    g->use_node = func;
}

/* Visit nodes and return the last one visited */
static vnode *
vg_visit(vgraph *g, vnode *from, vnode *to, int type, vlist *visit)
{
    static vqueue *queue = NULL;
    vlink *l, *lnext;
    double dist;
    vnode *n;

    /* Initialise */
    vq_init(queue);

    /* Set up initial links in queue */
    for (l = from->thead; l != NULL; l = l->tnext) {
        if (USELINK(g, l)) {
            /* Get distance */
            switch (type) {
            case V_DEPTH:
                dist = -1.0;
                break;
            case V_BREADTH:
                dist = 1.0;
                break;
            case V_PRIORITY:
                dist = vg_link_size(g, l);
                break;
            }

            /* Add link */
            SETDIST(l, dist);
            vq_pstore(queue, l, -dist);

            /* Flag destination node as looked-at */
            LOOKAT(l->to);
            SETPATH(l->to, l);
        }
    }

    /* Flag start node as visited */
    VISIT(from);
    if (visit != NULL)
        vl_spush(visit, from->name);
    from->path = NULL;
    if (cache_flag)
        from->cache = NULL;

    /* Do search */
    n = from;
    while (1) {
        /* If no more links, that's it */
        if ((l = vq_pget(queue)) == NULL)
            break;

        /* Skip link if destination node has been visited */
        n = l->to;
        if (VISITED(n))
            continue;

        /* Skip link if destination node can't be used */
        if (!USENODE(g, n, l->dist))
            continue;

        if (V_DEBUG(V_DBG_INTERN)) {
            vlist *path = vg_build_path(g, n);
            fprintf(stderr, "Moving %s -> %s (dist %g) [%s]",
                    l->from->name, l->to->name, l->dist,
                    vl_join(path, " -> "));
            vl_destroy(path);
        }

        /* Check for priority-based link shuffling */
        if (type == V_PRIORITY && SEEN(n) && n->path->dist > l->dist) {
            if (V_DEBUG(V_DBG_INTERN)) {
                vlist *path;
                lnext = n->path;
                SETPATH(n, l);
                path = vg_build_path(g, n);
                fprintf(stderr, "\nReplacing %s -> %s (dist %g) with %s -> %s (dist %g) [%s]\n",
                        lnext->from->name, lnext->to->name, lnext->dist,
                        l->from->name, l->to->name, l->dist,
                        vl_join(path, " -> "));
                vl_destroy(path);
            }

            SETPATH(n, l);
        }

        /* Flag node as visited */
        VISIT(n);
        if (visit != NULL)
            vl_spush(visit, n->name);

        /* If target node reached, that's it */
        if (n == to)
            break;

        /* Add node links to list */
        for (lnext = l->to->thead; lnext != NULL; lnext = lnext->tnext) {
            /* Skip if destination node visited */
            if (VISITED(lnext->to))
                continue;

            /* Skip link if it can't be used */
            if (!USELINK(g, lnext))
                continue;

            /* Get link distance */
            switch (type) {
            case V_DEPTH:
                dist = -1.0;
                break;
            case V_BREADTH:
                dist = 1.0;
                break;
            case V_PRIORITY:
                dist = vg_link_size(g, lnext);
                break;
            }

            /* Add link */
            SETDIST(lnext, l->dist + dist);
            vq_pstore(queue, lnext, -lnext->dist);

            /* Flag destination node as looked-at if required */
            if (!SEEN(lnext->to)) {
                LOOKAT(lnext->to);
                SETPATH(lnext->to, lnext);
            }

            if (V_DEBUG(V_DBG_INTERN))
                fprintf(stderr, "   Looking at %s -> %s (dist %g)",
                        lnext->from->name, lnext->to->name, lnext->dist);
        }
    }

    /* If destination not found, fail */
    if (to != NULL && n != to)
        return NULL;

    return n;
}

/* Write graph to a stream */
int
vg_write(vgraph *g, FILE *fp)
{
    int num = 0;
    vnode *n;
    vlink *l;

    VG_CHECK(g);

    /* Write nodes */
    if (!v_write_long(g->nodes, fp))
        return 0;

    for (n = g->nhead; n != NULL; n = n->nnext) {
        if (!v_write_string(n->name, fp))
            return 0;
        if (!vs_write(n->val, fp))
            return 0;
        n->pnum = num++;
    }

    /* Write links */
    if (!v_write_long(g->links, fp))
        return 0;

    for (l = g->lhead; l != NULL; l = l->lnext) {
        if (!v_write_long(l->from->pnum, fp))
            return 0;
        if (!v_write_long(l->to->pnum, fp))
            return 0;
        if (!vs_write(l->val, fp))
            return 0;
    }

    return 1;
}

/* Dump contents of a graph in XML format */
static int
vg_xmldump(vgraph *g, FILE *fp)
{
    char *from, *to, *oneway;
    vlink *l;
    vnode *n;

    VG_CHECK(g);

    /* Get usage information */
    vg_getusage(g);

    v_xmldump_start(fp);

    /* Dump nodes */
    for (n = g->nhead; n != NULL; n = n->nnext) {
        if (n->used && !vs_defined(n->val))
            continue;

        if (vs_defined(n->val)) {
            v_xmldump_tag_start(fp, "node", "name", n->name, NULL);

            if (vs_xmldump(n->val, fp))
                v_xmldump_tag_finish(fp, "node");
            else
                return 0;
        } else {
            v_xmldump_tag(fp, "node", "name", n->name, NULL);
        }
    }

    /* Dump links */
    for (l = g->lhead; l != NULL; l = l->lnext) {
        if (l->opposite)
            continue;

        from = l->from->name;
        to = l->to->name;
        oneway = l->oneway ? "true" : "false";

        if (vs_defined(l->val)) {
            v_xmldump_tag_start(fp, "link", "from", from,
                                "to", to, "oneway", oneway, NULL);

            if (vs_xmldump(l->val, fp))
                v_xmldump_tag_finish(fp, "link");
            else
                return 0;
        } else {
            v_xmldump_tag(fp, "link", "from", from,
                          "to", to, "oneway", oneway, NULL);
        }
    }

    v_xmldump_finish(fp);

    return 1;
}
