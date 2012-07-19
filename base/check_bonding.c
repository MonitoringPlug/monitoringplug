/***
 * Monitoring Plugin - check_bonding.c
 **
 *
 * check_bonding - Check bonding status.
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

const char *progname  = "check_bonding";
const char *progdesc  = "Check bonding status.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--bond=BOND]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

/* Structs */
struct bonding_slave_info_s {
    char            *interface;
    unsigned short  mii_status;
};
typedef struct bonding_slave_info_s bonding_slave_info;

struct bonding_info_s {
    char            *version;
    char            *mode;
    unsigned short  mii_status;
    bonding_slave_info  **slave;
    unsigned short      slaves;
};
typedef struct bonding_info_s bonding_info;

/* Global Vars */
char **bond;
int bonds = 0;

/* Function prototype */
bonding_info *parseBond(const char *filename);

int main (int argc, char **argv) {
    /* Local Vars */
    int             i;
    int             j;
    char            *filename;
    bonding_info    *info;
    char            *buf;
    char            *output = NULL;
    int             status = STATE_OK;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    if (bonds==0) {
        DIR *dir;
        struct dirent   *entry;
        dir = opendir("/proc/net/bonding/");
        if (dir == NULL)
            critical("Can't open '/proc/net/bonding/'");

        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, "bond", 4) == 0) {
                mp_array_push(&bond, strdup(entry->d_name), &bonds);
            }
        }

        closedir(dir);
    }

    filename = mp_malloc(sizeof(char)*32);
    buf = mp_malloc(sizeof(char)*64);

    for(i=0; i < bonds; i++) {
        mp_sprintf(filename, "/proc/net/bonding/%s", bond[i]);
        info = parseBond(filename);
        if (info == NULL) {
            status = STATE_CRITICAL;
            mp_snprintf(buf, 64, "%s not found", bond[i]);
            mp_strcat_comma(&output, buf);
        } else if (info->mii_status) {
            char *up = NULL;
            char *down = NULL;
            for(j=0; j < info->slaves; j++) {
                if (info->slave[j]->mii_status) {
                    mp_strcat_comma(&up, info->slave[j]->interface);
                } else {
                    mp_strcat_comma(&down, info->slave[j]->interface);
                }
            }
            if (down) {
                mp_snprintf(buf, 64, "%s up (%s, down: %s)", bond[i], up, down);
                status = status == STATE_OK ? STATE_WARNING : status;
            } else {
                mp_snprintf(buf, 64, "%s up (%s)", bond[i], up);
            }
            mp_strcat_comma(&output, buf);
        } else {
            mp_snprintf(buf, 64, "%s down (%s)", bond[i], info->mode);
            mp_strcat_comma(&output, buf);
            status = STATE_CRITICAL;
        }

    }

    switch (status) {
        case STATE_OK:
            ok(output);
        case STATE_WARNING:
            warning(output);
        case STATE_CRITICAL:
            critical(output);
        case STATE_UNKNOWN:
            unknown(output);
    }

    critical("You should never reach this point.");
}

bonding_info *parseBond(const char *filename) {
    FILE *input;
    char buffer[256];
    char *key, *value;
    int count = 0;

    bonding_info *info;

    input = fopen(filename, "r");
    if (input == NULL)
        return NULL;

    info = mp_malloc(sizeof(bonding_info));
    info->slave = NULL;

    while (fgets(buffer, 256, input) != NULL) {

        value = buffer;
        key = strsep(&value, ":");
        if(!value)
            continue;

        value++;
        value[strlen(value)-1] = '\0';

        if (strcmp(key, "Ethernet Channel Bonding Driver") == 0) {
            info->version = strdup(value);
        } else if (strcmp(key, "Bonding Mode") == 0) {
            info->mode = strdup(value);
        } else if (strcmp(key, "MII Status") == 0) {
            if (strcmp(value, "up") == 0) {
                if(count)
                    info->slave[count-1]->mii_status = 1;
                else
                    info->mii_status = 1;
            } else {
                if(count)
                    info->slave[count-1]->mii_status = 0;
                else
                    info->mii_status = 0;
            }
        } else if (strcmp(key, "Slave Interface") == 0) {
            count++;
            info->slaves = count;
            info->slave = mp_realloc(info->slave, (count+1)*sizeof(bonding_slave_info *));
            info->slave[count] = NULL;
            info->slave[count-1] = mp_malloc(sizeof(struct bonding_slave_info_s));
            info->slave[count-1]->interface = strdup(value);
        }
    }
    fclose(input);

    return info;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_WC,
        {"bonding", required_argument, NULL, (int)'b'},
        MP_LONGOPTS_END,
    };

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"b:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch (c) {
            case 'b':
                mp_array_push(&bond, strdup(optarg), &bonds);
                break;
        }
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    printf(" -b, --bond=BOND\n");
    printf("      Check bond named BOND.\n");
}

/* vim: set ts=4 sw=4 et syn=c : */
