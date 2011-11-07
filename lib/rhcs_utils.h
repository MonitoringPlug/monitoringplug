/***
 * Monitoring Plugin - rhcs_utils.h
 **
 *
 * Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

#ifndef RHCS_UTILS_H_
#define RHCS_UTILS_H_

#include <stdio.h>

struct rhcs_clustat_node_s {
    char                *name;
    unsigned short      state;
    unsigned short      rgmanager;
    unsigned short      id;
};
typedef struct rhcs_clustat_node_s rhcs_clustat_node;

struct rhcs_clustat_group_s {
    char                *name;
    int                 state;
    rhcs_clustat_node   *owner;
    rhcs_clustat_node   *last;
};
typedef struct rhcs_clustat_group_s rhcs_clustat_group;

struct rhcs_clustat_s {
    char                *name;
    int                 id;
    rhcs_clustat_node   *local;
    rhcs_clustat_node   **node;
    rhcs_clustat_group  **group;
};
typedef struct rhcs_clustat_s rhcs_clustat;

rhcs_clustat *parse_rhcs_clustat(FILE *in);
void rhcs_clustat_startElement(void *clustat, const char *name, const char **atts);
void rhcs_clustat_stopElement(void *clustat, const char *name);

enum {
    RESTART,
    RELOCATE,
};

struct rhcs_conf_node_s {
    char                *name;
    unsigned int        id;
    unsigned int        votes;
};
typedef struct rhcs_conf_node_s rhcs_conf_node;


struct rhcs_conf_fodom_node_s {
    rhcs_conf_node      *node;
    unsigned int        priority;
};
typedef struct rhcs_conf_fodom_node_s rhcs_conf_fodom_node;

struct rhcs_conf_fodom_s {
    char                *name;
    unsigned short      failback;
    unsigned short      ordered;
    unsigned short      restricted;
    unsigned int        nodes;
    rhcs_conf_fodom_node    **node;
};
typedef struct rhcs_conf_fodom_s rhcs_conf_fodom;

struct rhcs_conf_service_s {
    char                *name;
    rhcs_conf_fodom     *fodomain;
    unsigned short      recovery;
    unsigned short      autostart;
    unsigned short      exclusive;
};
typedef struct rhcs_conf_service_s rhcs_conf_service;

struct rhcs_conf_s {
    char                *name;
    char                *alias;
    unsigned int        version;
    rhcs_conf_node      **node;
    rhcs_conf_fodom     **fodomain;
    rhcs_conf_service   **service;
};
typedef struct rhcs_conf_s rhcs_conf;

rhcs_conf *parse_rhcs_conf(FILE *in);
void rhcs_conf_startElement(void *conf, const char *name, const char **atts);
void rhcs_conf_stopElement(void *conf, const char *name);


#endif /* RHCS_UTILS_H_ */
