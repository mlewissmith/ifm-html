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
  @brief Graph functions and macros.
  @ingroup graph
*/

#ifndef VARS_GRAPH_H
#define VARS_GRAPH_H

#include <vars-list.h>

/*! @brief Abort if a pointer is not a graph. */
#define VG_CHECK(ptr)            V_CHECK(ptr, vg_check, "GRAPH")

/*! @brief Create a node with an undefined value. */
#define vg_node(g, n)            vg_node_store(g, n, vs_create(V_TYPE_UNDEF))

/*! @brief Get the integer value of a node. */
#define vg_node_iget(g, n)       vs_iget(vg_node_get(g, n))

/*! @brief Create a node with an integer value. */
#define vg_node_istore(g, n, v)  vg_node_store(g, n, vs_icreate(v))

/*! @brief Get the float value of a node. */
#define vg_node_fget(g, n)       vs_fget(vg_node_get(g, n))

/*! @brief Create a node with a float value. */
#define vg_node_fstore(g, n, v)  vg_node_store(g, n, vs_fcreate(v))

/*! @brief Get the double value of a node. */
#define vg_node_dget(g, n)       vs_dget(vg_node_get(g, n))

/*! @brief Create a node with a double value. */
#define vg_node_dstore(g, n, v)  vg_node_store(g, n, vs_dcreate(v))

/*! @brief Get the string value of a node. */
#define vg_node_sget(g, n)       vs_sget(vg_node_get(g, n))

/*! @brief Create a node with a string value. */
#define vg_node_sstore(g, n, v)  vg_node_store(g, n, vs_screate(v))

/*! @brief Get the pointer value of a node. */
#define vg_node_pget(g, n)       vs_pget(vg_node_get(g, n))

/*! @brief Create a node with a pointer value. */
#define vg_node_pstore(g, n, v)  vg_node_store(g, n, vs_pcreate(v))

/*! @brief Create a two-way link with an undefined value. */
#define vg_link(g, n1, n2) \
        vg_link_store(g, n1, n2, vs_create(V_TYPE_UNDEF))

/*! @brief Create a one-way link with an undefined value. */
#define vg_link_oneway(g, n1, n2) \
        vg_link_oneway_store(g, n1, n2, vs_create(V_TYPE_UNDEF))

/*! @brief Get the integer value of a link. */
#define vg_link_iget(g, n1, n2) \
        vs_iget(vg_link_get(g, n1, n2))

/*! @brief Create a two-way integer-valued link. */
#define vg_link_istore(g, n1, n2, v) \
        vg_link_store(g, n1, n2, vs_icreate(v))

/*! @brief Create a one-way integer-valued link. */
#define vg_link_oneway_istore(g, n1, n2, v) \
        vg_link_oneway_store(g, n1, n2, vs_icreate(v))

/*! @brief Get the float value of a link. */
#define vg_link_fget(g, n1, n2) \
        vs_fget(vg_link_get(g, n1, n2))

/*! @brief Create a two-way float-valued link. */
#define vg_link_fstore(g, n1, n2, v) \
        vg_link_store(g, n1, n2, vs_fcreate(v))

/*! @brief Create a one-way float-valued link. */
#define vg_link_oneway_fstore(g, n1, n2, v) \
        vg_link_oneway_store(g, n1, n2, vs_fcreate(v))

/*! @brief Get the double value of a link. */
#define vg_link_dget(g, n1, n2) \
        vs_dget(vg_link_get(g, n1, n2))

/*! @brief Create a two-way double-valued link. */
#define vg_link_dstore(g, n1, n2, v) \
        vg_link_store(g, n1, n2, vs_dcreate(v))

/*! @brief Create a one-way double-valued link. */
#define vg_link_oneway_dstore(g, n1, n2, v) \
        vg_link_oneway_store(g, n1, n2, vs_dcreate(v))

/*! @brief Get the string value of a link. */
#define vg_link_sget(g, n1, n2) \
        vs_sget(vg_link_get(g, n1, n2))

/*! @brief Create a two-way string-valued link. */
#define vg_link_sstore(g, n1, n2, v) \
        vg_link_store(g, n1, n2, vs_screate(v))

/*! @brief Create a one-way string-valued link. */
#define vg_link_oneway_sstore(g, n1, n2, v) \
        vg_link_oneway_store(g, n1, n2, vs_screate(v))

/*! @brief Get the pointer value of a link. */
#define vg_link_pget(g, n1, n2) \
        vs_pget(vg_link_get(g, n1, n2))

/*! @brief Create a two-way pointer-valued link. */
#define vg_link_pstore(g, n1, n2, v) \
        vg_link_store(g, n1, n2, vs_pcreate(v))

/*! @brief Create a one-way pointer-valued link. */
#define vg_link_oneway_pstore(g, n1, n2, v) \
        vg_link_oneway_store(g, n1, n2, vs_pcreate(v))

/*! @brief Graph type. */
typedef struct v_graph vgraph;

#ifdef __cplusplus
extern "C" {
#endif

extern int vg_caching(void);
extern int vg_check(void *ptr);
extern vlist *vg_connected(vgraph *g);
extern vgraph *vg_copy(vgraph *g);
extern vgraph *vg_create(void);
extern vtype *vg_declare(void);
extern void vg_delete(vgraph *g, char *node);
extern void vg_destroy(vgraph *g);
extern int vg_freeze(vgraph *g, FILE *fp);
extern int vg_link_count(vgraph *g);
extern int vg_link_exists(vgraph *g, char *node1, char *node2);
extern vscalar *vg_link_get(vgraph *g, char *node1, char *node2);
extern void vg_link_oneway_store(vgraph *g, char *node1, char *node2,
                                 vscalar *s);
extern void vg_link_size_function(vgraph *g, double (*func)(char *node1,
                                  char *node2, vscalar *s));
extern void vg_link_store(vgraph *g, char *node1, char *node2, vscalar *s);
extern int vg_node_count(vgraph *g);
extern int vg_node_exists(vgraph *g, char *node);
extern vlist *vg_node_from(vgraph *g, char *node);
extern vscalar *vg_node_get(vgraph *g, char *node);
extern int vg_node_links(vgraph *g, char *node, int *from, int *to);
extern vlist *vg_node_list(vgraph *g);
extern void vg_node_store(vgraph *g, char *node, vscalar *s);
extern vlist *vg_node_to(vgraph *g, char *node);
extern double vg_path_cache(vgraph *g, char *node);
extern int vg_path_exists(vgraph *g, char *node1, char *node2);
extern vlist *vg_path_info(vgraph *g, char *node1, char *node2);
extern double vg_path_length(vgraph *g, char *node1, char *node2);
extern vlist *vg_path_links(vgraph *g, char *from, char *to);
extern vlist *vg_path_nodes(vgraph *g, char *node1, char *node2);
extern vlist *vg_path_reachable(vgraph *g, char *node);
extern void vg_print(vgraph *g, FILE *fp);
extern vgraph *vg_read(FILE *fp);
extern vgraph *vg_thaw(FILE *fp);
extern int vg_traverse(vgraph *g, int (*func)(void *ptr));
extern vlist *vg_tsort(vgraph *g);
extern vlist *vg_tsort_cycles(void);
extern void vg_unlink(vgraph *g, char *node1, char *node2);
extern void vg_unlink_oneway(vgraph *g, char *node1, char *node2);
extern void vg_use_cache(vgraph *g, int flag);
extern void vg_use_link_function(vgraph *g, int (*func)(char *node1,
                                 char *node2, vscalar *s));
extern void vg_use_node_function(vgraph *g, int (*func)(char *node,
                                 vscalar *s, double dist));
extern int vg_write(vgraph *g, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
