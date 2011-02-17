/***
 * Monitoring Plugin - mp_eopt.c
 **
 *
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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

#include "mp_common.h"
#include "mp_eopt.h"

#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char **mp_eopt(int *argc, char **orig_argv, char *optarg) {
    FILE *fd;
    char *buffer;
    char *efile;
    char *esection;
    char **eargv;
    char *arg;
    int i = 0;

    if (optarg == NULL && strncmp(orig_argv[optind], "-",1) != 0) {
        optarg = orig_argv[optind];
        optind++;
    }

    efile = "/etc/nagios/monitoringplug.ini";
    esection = (char *)progname;

    // Parse optarg if available
    // [section][@file]
    if (optarg) {
        if(optarg[0] == '@') {
            efile = optarg+1;
        } else {
            esection = strsep(&optarg, "@");
            if (optarg)
                efile = optarg;
        }
    }

    fd = fopen(efile, "r");
    if (!fd) {
        printf("Can't open: %s\n", efile);
        return orig_argv;
    }

    buffer = malloc(256);
    int lineno = 0;
    int len;
    int current_section = 0;
    char **new_argv=NULL;
    int new_argc = 0;

    while (fgets(buffer, 256, fd) != NULL) {
        lineno++;

        len = (int)strlen(buffer) - 1;

        // Test if line is complete.
        if (buffer[len] != '\n') {
            printf("Line %d in file %s too long.\n", lineno, efile);
            return orig_argv;
        }

        // R-Trim
        while (len>=0 && (isspace(buffer[len]) || buffer[len] == '\n')) {
            buffer[len] = 0;
            len--;
        }

        if (len < 1) {
            // Empty Line
            continue;
        } else if (buffer[0]=='#' || buffer[0]==';') {
            //Comment
            continue;
        } else if (buffer[0]=='[' && buffer[len]==']') {
            buffer[len] = 0;
            if (strcmp(esection, buffer+1) == 0)
                current_section = 1;
            else
                current_section = 0;
            continue;
        } else if(current_section == 0) {
            // Ignore other sections
            continue;
        } else {
            char *key, *val;
            val = buffer;
            key = strsep(&val, "=");

            if(*val) {
                new_argv = realloc(new_argv, sizeof(char *)*(new_argc+2));
            } else {
                new_argv = realloc(new_argv, sizeof(char *)*(new_argc+1));
            }

            len = strlen(key);
            if (len > 1) {
                arg = malloc(len+3);
                sprintf(arg, "--%s", key);
            } else {
                arg = malloc(len+2);
                sprintf(arg, "-%s", key);
            }

            new_argv[new_argc] = arg;
            new_argc++;

            if(*val) {
                new_argv[new_argc] = strdup(val);
                new_argc++;
            }
        }
    }

    eargv = (char**)malloc(sizeof(char *)*(*argc+new_argc));
    for (i=0; i<optind; i++) eargv[i]=orig_argv[i];
    for (i=0;i<new_argc;i++) eargv[optind+i]=new_argv[i];
    for (i=optind; i<*argc; i++) eargv[new_argc+i]=orig_argv[i];

    *argc += new_argc;

    return eargv;
}

void print_help_eopt(void) {
    printf("\
 -E, --eopt=[section][@file]\n\
      Read additional opts from section in ini-File.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */
