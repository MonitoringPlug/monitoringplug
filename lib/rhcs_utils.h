/***
 * Monitoring Plugin - rhcs_utils.h
 **
 *
 * Copyright (C) 2012 Marius Rieder <marius.rieder@durchmesser.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * $Id$
 */

#ifndef RHCS_UTILS_H_
#define RHCS_UTILS_H_

#include <stdio.h>

/** Restart policy enum. */
enum {
    RESTART,    /**< Restart service inpace. */
    RELOCATE,   /**< Relocate service according to failover domain. */
};

/** RHCS clustat node struct */
struct rhcs_clustat_node_s {
    char                *name;      /**< Node name. */
    unsigned short      state;      /**< Node state. */
    unsigned short      rgmanager;  /**< rgmanager status */
    unsigned short      id;         /**< Node id. */
};
typedef struct rhcs_clustat_node_s rhcs_clustat_node;

/** RHCS clustat group struct */
struct rhcs_clustat_group_s {
    char                *name;      /**< Group name. */
    int                 state;      /**< Group state. */
    rhcs_clustat_node   *owner;     /**< Current owner node. */
    rhcs_clustat_node   *last;      /**< Last owner node. */
};
typedef struct rhcs_clustat_group_s rhcs_clustat_group;

/** RHCS clustat struct */
struct rhcs_clustat_s {
    char                *name;      /**< Cluster name. */
    int                 id;         /**< Cluster id. */
    rhcs_clustat_node   *local;     /**< Local node. */
    rhcs_clustat_node   **node;     /**< List of nodes. */
    rhcs_clustat_group  **group;    /**< List of groups. */
};
typedef struct rhcs_clustat_s rhcs_clustat;


/** RHCS cluster config node struct */
struct rhcs_conf_node_s {
    char                *name;      /**< Node name. */
    unsigned int        id;         /**< Node id. */
    unsigned int        votes;      /**< Number of votes. */
};
typedef struct rhcs_conf_node_s rhcs_conf_node;

/** RHCS cluster config failover domain entry struct */
struct rhcs_conf_fodom_node_s {
    rhcs_conf_node      *node;      /**< FD entry node. */
    unsigned int        priority;   /**< FD entry protity. */
};
typedef struct rhcs_conf_fodom_node_s rhcs_conf_fodom_node;

/** RHCS cluster config failover domain struct */
struct rhcs_conf_fodom_s {
    char                *name;      /**< Failover Domain name */
    unsigned short      failback;   /**< Fallback on recovery. */
    unsigned short      ordered;    /**< Ordered Failover Domain. */
    unsigned short      restricted; /**< Restricted Failover Domain. */
    unsigned int        nodes;      /**< Number of member nodes. */
    rhcs_conf_fodom_node    **node; /**< List of members. */
};
typedef struct rhcs_conf_fodom_s rhcs_conf_fodom;

/** RHCS cluster config service struct */
struct rhcs_conf_service_s {
    char                *name;      /**< Service name. */
    rhcs_conf_fodom     *fodomain;  /**< Service failover domain. */
    unsigned short      recovery;   /**< Recovery vs. relocate. */
    unsigned short      autostart;  /**< Autostart service. */
    unsigned short      exclusive;  /**< Exclusive service. */
};
typedef struct rhcs_conf_service_s rhcs_conf_service;

/** RHCS cluster config struct */
struct rhcs_conf_s {
    char                *name;      /**< Cluster name. */
    char                *alias;     /**< Cluster alias. */
    unsigned int        version;    /**< Cluster config revision. */
    rhcs_conf_node      **node;     /**< List of nodes. */
    rhcs_conf_fodom     **fodomain; /**< List of failover domains. */
    rhcs_conf_service   **service;  /**< List of services. */
};
typedef struct rhcs_conf_s rhcs_conf;

/**
 * Parse the clustat XML output.
 * \para[in] in FILE pointer to read from.
 * \return Return a rhcs_clustat struct.
 */
rhcs_clustat *parse_rhcs_clustat(FILE *in);

/**
 * eXpat startElement callback for parsing clustat.
 * \para[in|out] clustat rhcs_clustat struct to add infos to.
 * \para[in] name Element name.
 * \para[in] atts Element attributes.
 */
void rhcs_clustat_startElement(void *clustat, const char *name, const char **atts);

/**
 * eXpat stopElement callback for parsing clustat.
 * \para[in|out] clustat rhcs_clustat struct to add infos to.
 * \para[in] name Element name.
 */
void rhcs_clustat_stopElement(void *clustat, const char *name);

/**
 * Parse the cluster.conf.
 * \para[in] in FILE pointer to read from.
 * \return Return a rhcs_conf struct.
 */
rhcs_conf *parse_rhcs_conf(FILE *in);

/**
 * eXpat startElement callback for parsing cluster.conf.
 * \para[in|out] conf rhcs_conf struct to add infos to.
 * \para[in] name Element name.
 * \para[in] atts Element attributes.
 */
void rhcs_conf_startElement(void *conf, const char *name, const char **atts);

/**
 * eXpat stopElement callback for parsing cluster.conf.
 * \para[in|out] conf rhcs_conf struct to add infos to.
 * \para[in] name Element name.
 */
void rhcs_conf_stopElement(void *conf, const char *name);

#endif /* RHCS_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
