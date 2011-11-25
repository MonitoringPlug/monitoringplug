/***
 * Monitoring Plugin - rhcs_utils.c
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

#include "rhcs_utils.h"

#include "mp_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <expat.h>

#define BUFFERSIZE 1024


int nodes;
int fodomains;
int fodomain_nodes;
int services;

rhcs_clustat *parse_rhcs_clustat(FILE *in) {
    char *buf;
    size_t len;
    int done = 0;
    XML_Parser parser;
    rhcs_clustat *clustat;

    nodes = 0;
    services = 0;

    clustat = mp_calloc(1, sizeof(rhcs_clustat));

    // Create Parser
    parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, clustat);
    XML_SetElementHandler(parser, rhcs_clustat_startElement, rhcs_clustat_stopElement);

    buf = mp_calloc(BUFFERSIZE, 1);

    do {
       len = fread(buf, 1, BUFFERSIZE, in);
       if (!XML_Parse(parser, buf, len, done)) {
          fprintf(stderr,
             "%s at line %d\n",
             XML_ErrorString(XML_GetErrorCode(parser)),
             (int) XML_GetCurrentLineNumber(parser));
          return NULL;
       }
    } while (len == BUFFERSIZE && done == 0);
    XML_ParserFree(parser);

    return clustat;
}
void rhcs_clustat_startElement(void *clustat, const char *name, const char **atts) {
    const char **k, **v;
    int i;

    if (strcmp(name, "cluster") == 0) {
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "name") == 0)
                ((rhcs_clustat *)clustat)->name = strdup(*v);
            else if (strcmp(*k, "id") == 0)
                ((rhcs_clustat *)clustat)->id = (unsigned int) strtol(*v, NULL, 10);
        }
        return;
    }

    if (strcmp(name, "node") == 0) {
        ((rhcs_clustat *)clustat)->node = mp_realloc(((rhcs_conf *)clustat)->node, (nodes+2)*sizeof(rhcs_clustat_node));
        ((rhcs_clustat *)clustat)->node[nodes] = mp_calloc(1, sizeof(rhcs_clustat_node));
        ((rhcs_clustat *)clustat)->node[nodes+1] = NULL;
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "name") == 0)
                ((rhcs_clustat *)clustat)->node[nodes]->name = strdup(*v);
            else if (strcmp(*k, "state") == 0)
                ((rhcs_clustat *)clustat)->node[nodes]->state = (unsigned int) strtol(*v, NULL, 10);
            else if (strcmp(*k, "rgmanager") == 0)
                ((rhcs_clustat *)clustat)->node[nodes]->rgmanager = (unsigned int) strtol(*v, NULL, 10);
            else if (strcmp(*k, "nodeid") == 0)
                ((rhcs_clustat *)clustat)->node[nodes]->id = (unsigned int) strtol(*v, NULL, 16);
            if (strcmp(*k, "local") == 0 && strcmp(*v, "1") == 0)
                ((rhcs_clustat *)clustat)->local = ((rhcs_clustat *)clustat)->node[nodes];
        }
        nodes++;
        return;
    }

    if (strcmp(name, "group") == 0) {
        ((rhcs_clustat *)clustat)->group = mp_realloc(((rhcs_clustat *)clustat)->group, (services+2)*sizeof(rhcs_clustat_group));
        ((rhcs_clustat *)clustat)->group[services] = mp_calloc(1, sizeof(rhcs_clustat_group));
        ((rhcs_clustat *)clustat)->group[services+1] = NULL;
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "name") == 0) {
                ((rhcs_clustat *)clustat)->group[services]->name = strdup(*v);
                strsep(&((rhcs_clustat *)clustat)->group[services]->name, ":");
            } else if (strcmp(*k, "state") == 0) {
                ((rhcs_clustat *)clustat)->group[services]->state = (unsigned int) strtol(*v, NULL, 10);
	    } else if (strcmp(*k, "owner") == 0) {
	        ((rhcs_clustat *)clustat)->group[services]->owner = NULL;
                for(i=0; i < nodes; i++) {
                    if(strcmp(*v, ((rhcs_clustat *)clustat)->node[i]->name) == 0) {
                        ((rhcs_clustat *)clustat)->group[services]->owner = ((rhcs_clustat *)clustat)->node[i];
                    }
                }
	    } else if (strcmp(*k, "last_owner") == 0) {
                for(i=0; i < nodes; i++) {
                    if(strcmp(*v, ((rhcs_clustat *)clustat)->node[i]->name) == 0) {
                        ((rhcs_clustat *)clustat)->group[services]->last = ((rhcs_clustat *)clustat)->node[i];
                    }
                }
	    }
        }
        services++;
        return;
    }
}
void rhcs_clustat_stopElement(void *clustat, const char *name) {

}

rhcs_conf *parse_rhcs_conf(FILE *in) {
    char *buf;
    size_t len;
    int done = 0;
    XML_Parser parser;
    rhcs_conf *conf;

    nodes = 0;
    services = 0;

    conf = mp_calloc(1, sizeof(rhcs_conf));

    // Create Parser
    parser = XML_ParserCreate(NULL);

    // Setup Parser
    XML_SetUserData(parser, conf);
    XML_SetElementHandler(parser, rhcs_conf_startElement, rhcs_conf_stopElement);

    buf = mp_calloc(BUFFERSIZE, 1);

    do {
       len = fread(buf, 1, BUFFERSIZE, in);
       if (!XML_Parse(parser, buf, len, done)) {
          fprintf(stderr,
             "%s at line %d\n",
             XML_ErrorString(XML_GetErrorCode(parser)),
             (int) XML_GetCurrentLineNumber(parser));
          return NULL;
       }
    } while (len == BUFFERSIZE && done == 0);
    XML_ParserFree(parser);

    return conf;
}

void rhcs_conf_startElement(void *conf, const char *name, const char **atts) {
    const char **k, **v;
    int i;

    if (strcmp(name, "cluster") == 0) {
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "name") == 0)
                ((rhcs_conf *)conf)->name = strdup(*v);
            else if (strcmp(*k, "alias") == 0)
                ((rhcs_conf *)conf)->alias = strdup(*v);
            else if (strcmp(*k, "config_version") == 0)
                ((rhcs_conf *)conf)->version = (unsigned int) strtol(*v, NULL, 10);
        }
        return;
    }
    if (strcmp(name, "clusternode") == 0) {
        ((rhcs_conf *)conf)->node = mp_realloc(((rhcs_conf *)conf)->node, (nodes+2)*sizeof(rhcs_conf_node));
        ((rhcs_conf *)conf)->node[nodes] = mp_calloc(1, sizeof(rhcs_conf_node));
        ((rhcs_conf *)conf)->node[nodes+1] = NULL;
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "name") == 0)
                ((rhcs_conf *)conf)->node[nodes]->name = strdup(*v);
            else if (strcmp(*k, "nodeid") == 0)
                ((rhcs_conf *)conf)->node[nodes]->id = (unsigned int) strtol(*v, NULL, 10);
            else if (strcmp(*k, "votes") == 0)
                ((rhcs_conf *)conf)->node[nodes]->votes = (unsigned int) strtol(*v, NULL, 10);
        }
        nodes++;
        return;
    }
    if (strcmp(name, "failoverdomain") == 0) {
        ((rhcs_conf *)conf)->fodomain = mp_realloc(((rhcs_conf *)conf)->fodomain, (fodomains+2)*sizeof(rhcs_conf_fodom));
        ((rhcs_conf *)conf)->fodomain[fodomains] = mp_calloc(1, sizeof(rhcs_conf_fodom));
        ((rhcs_conf *)conf)->fodomain[fodomains+1] = NULL;
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "name") == 0)
                ((rhcs_conf *)conf)->fodomain[fodomains]->name = strdup(*v);
            else if (strcmp(*k, "nofailback") == 0)
                ((rhcs_conf *)conf)->fodomain[fodomains]->failback = (strcmp(*v, "0") == 0);
            else if (strcmp(*k, "ordered") == 0)
                ((rhcs_conf *)conf)->fodomain[fodomains]->ordered = (strcmp(*v, "1") == 0);
            else if (strcmp(*k, "restricted") == 0)
                ((rhcs_conf *)conf)->fodomain[fodomains]->restricted = (strcmp(*v, "1") == 0);
        }
        fodomain_nodes = 0;
        return;
    }
    if (strcmp(name, "failoverdomainnode") == 0) {
        ((rhcs_conf *)conf)->fodomain[fodomains]->node = mp_realloc(((rhcs_conf *)conf)->fodomain[fodomains]->node, (fodomain_nodes+2)*sizeof(rhcs_conf_fodom_node));
        ((rhcs_conf *)conf)->fodomain[fodomains]->node[fodomain_nodes] = mp_calloc(1, sizeof(rhcs_conf_fodom_node));
        ((rhcs_conf *)conf)->fodomain[fodomains]->node[fodomain_nodes+1] = NULL;
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "name") == 0)
                for (i=0; i < nodes; i++) {
                    if (strcmp(*v, ((rhcs_conf *)conf)->node[i]->name) == 0) {
                        ((rhcs_conf *)conf)->fodomain[fodomains]->node[fodomain_nodes]->node = ((rhcs_conf *)conf)->node[i];
                    }
                }
            else if (strcmp(*k, "priority") == 0)
                ((rhcs_conf *)conf)->fodomain[fodomains]->node[fodomain_nodes]->priority = (unsigned int) strtol(*v, NULL, 10);
        }
        fodomain_nodes++;
        return;
    }
    if (strcmp(name, "service") == 0) {
        ((rhcs_conf *)conf)->service = mp_realloc(((rhcs_conf *)conf)->service, (services+2)*sizeof(rhcs_conf_service));
        ((rhcs_conf *)conf)->service[services] = mp_calloc(1, sizeof(rhcs_conf_service));
        ((rhcs_conf *)conf)->service[services+1] = NULL;
        for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2) {
            if (strcmp(*k, "name") == 0)
                ((rhcs_conf *)conf)->service[services]->name = strdup(*v);
            else if (strcmp(*k, "domain") == 0)
                for (i=0; i < fodomains; i++) {
                    if (strcmp(*v, ((rhcs_conf *)conf)->fodomain[i]->name) == 0) {
                        ((rhcs_conf *)conf)->service[services]->fodomain = ((rhcs_conf *)conf)->fodomain[i];
                    }
                }
            else if (strcmp(*k, "autostart") == 0)
                ((rhcs_conf *)conf)->service[services]->autostart = (strcmp(*v, "1") == 0);
            else if (strcmp(*k, "exclusive") == 0)
                ((rhcs_conf *)conf)->service[services]->exclusive = (strcmp(*v, "1") == 0);
            else if (strcmp(*k, "recovery") == 0) {
                if (strcmp(*v, "relocate") == 0)
                    ((rhcs_conf *)conf)->service[services]->recovery = RELOCATE;
                else if (strcmp(*v, "restart") == 0)
                    ((rhcs_conf *)conf)->service[services]->recovery = RESTART;
            }
        }
        services++;
        return;
    }
/*
    printf("%s\n", name);

    for(k = atts, v = atts+1; *k != NULL; k+=2, v+=2)
        printf("  '%s' => '%s'\n", *k, *v);
*/
}
void rhcs_conf_stopElement(void *conf, const char *name) {
    if (strcmp(name, "failoverdomain") == 0)
        fodomains++;
}

/* vim: set ts=4 sw=4 et syn=c : */
