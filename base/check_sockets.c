/***
 * Monitoring Plugin - check_sockets
 **
 *
 * check_sockets - Check number of open Sockets.
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

const char *progname  = "check_sockets";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-f <FILE> [-w <warning age>] [-c <critical age>]";

#include "mp_common.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

/* Global vars */
int tcpport = -1;
int udpport = -1;
int rawport = -1;
int ipv4 = 1;
int ipv6 = 1;
thresholds *socket_thresholds = NULL;

/* function prototype */
int countSocket(const char *filename, int port);

int main (int argc, char **argv) {

    int         cnt;
    int         status = STATE_OK;
    int         lstatus;
    char        *output = NULL;

    if (process_arguments (argc, argv) == 1)
        exit(STATE_CRITICAL);

    if (tcpport < 0 && udpport < 0 && rawport < 0)
        usage("--tcp, --udp or --raw mandatory.");

    if (tcpport >=0) {
        if(ipv4 > 0) {
            cnt = countSocket("tcp",tcpport);
            lstatus = get_status(cnt, socket_thresholds);

            switch (lstatus) {
                case STATE_WARNING:
                    output = malloc(sizeof(char) * 12);
                    strcat(output, "tcp warning");
                    break;
                case STATE_CRITICAL:
                    output = malloc(sizeof(char) * 13);
                    strcat(output, "tcp critical");
                    break;
            }

            status = lstatus > status ? lstatus : status;
        }
        if(ipv6 > 0) {
            cnt = countSocket("tcp6",tcpport);
            lstatus = get_status(cnt, socket_thresholds);

            switch (lstatus) {
                case STATE_WARNING:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 15);
                        strcat(output, ", tcp6 warning");
                    } else {
                        output = malloc(sizeof(char) * 13);
                        strcat(output, "tcp6 warning");
                    }
                    break;
                case STATE_CRITICAL:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 16);
                        strcat(output, ", tcp6 critical");
                    } else {
                        output = malloc(sizeof(char) * 14);
                        strcat(output, "tcp6 critical");
                    }
                    break;
            }

            status = lstatus > status ? lstatus : status;
        }
    }
    if (udpport >=0) {
        if(ipv4 > 0) {
            cnt = countSocket("udp",udpport);
            lstatus = get_status(cnt, socket_thresholds);

            switch (lstatus) {
                case STATE_WARNING:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 14);
                        strcat(output, ", udp warning");
                    } else {
                        output = malloc(sizeof(char) * 12);
                        strcat(output, "udp warning");
                    }
                    break;
                case STATE_CRITICAL:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 15);
                        strcat(output, ", udp critical");
                    } else {
                        output = malloc(sizeof(char) * 13);
                        strcat(output, "udp critical");
                    }
                    break;
            }

            status = lstatus > status ? lstatus : status;
        }
        if(ipv6 > 0) {
            cnt = countSocket("udp6",udpport);
            lstatus = get_status(cnt, socket_thresholds);

            switch (lstatus) {
                case STATE_WARNING:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 15);
                        strcat(output, ", udp6 warning");
                    } else {
                        output = malloc(sizeof(char) * 13);
                        strcat(output, "udp6 warning");
                    }
                    break;
                case STATE_CRITICAL:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 16);
                        strcat(output, ", udp6 critical");
                    } else {
                        output = malloc(sizeof(char) * 14);
                        strcat(output, "udp6 critical");
                    }
                    break;
            }

            status = lstatus > status ? lstatus : status;
        }
    }
    if (rawport >=0) {
        if(ipv4 > 0) {
            cnt = countSocket("raw",rawport);
            lstatus = get_status(cnt, socket_thresholds);

            switch (lstatus) {
                case STATE_WARNING:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 14);
                        strcat(output, ", raw warning");
                    } else {
                        output = malloc(sizeof(char) * 12);
                        strcat(output, "raw warning");
                    }
                    break;
                case STATE_CRITICAL:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 15);
                        strcat(output, ", raw critical");
                    } else {
                        output = malloc(sizeof(char) * 13);
                        strcat(output, "raw critical");
                    }
                    break;
            }

            status = lstatus > status ? lstatus : status;
        }
        if(ipv6 > 0) {
            cnt = countSocket("raw6",rawport);
            lstatus = get_status(cnt, socket_thresholds);

            switch (lstatus) {
                case STATE_WARNING:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 15);
                        strcat(output, ", raw6 warning");
                    } else {
                        output = malloc(sizeof(char) * 13);
                        strcat(output, "raw6 warning");
                    }
                    break;
                case STATE_CRITICAL:
                    if (output != NULL) {
                        output = realloc(output, strlen(output) + sizeof(char) * 16);
                        strcat(output, ", raw6 critical");
                    } else {
                        output = malloc(sizeof(char) * 14);
                        strcat(output, "raw6 critical");
                    }
                    break;
            }

            status = lstatus > status ? lstatus : status;
        }
    }

    switch (status) {
        case STATE_OK:
            ok("Everithing ok.");
        case STATE_WARNING:
            warning(output);
        case STATE_CRITICAL:
            critical(output);
    }

    critical("You should never reach this point.");
}

int countSocket(const char *filename, int port) {
    FILE *input;
    char buffer[256];
    char *c, *next;
    int cnt = 0;

    input = fopen(filename, "r");

    fgets(buffer, 256, input);

    while (fgets(buffer, 256, input) != NULL) {
        if (port > 0) {
            next = buffer;
            c = strsep(&next, ":");
            c = strsep(&next, ":");
            c = strsep(&next, " ");
            if(port == strtol(c, NULL, 16)) {
                cnt++;
            }
        } else {
            cnt++;
        }
    }

    fclose(input);

    printf("%s: %d\n", filename, cnt);

    return cnt;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"tcp", required_argument, NULL, (int)'t'},
        {"udp", required_argument, NULL, (int)'u'},
        {"raw", required_argument, NULL, (int)'r'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:u:r:c:w:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_default(c);
        getopt_46(c, &ipv4, &ipv6);
        getopt_wc(c, optarg, &socket_thresholds);

        switch (c) {
            case 't':
                if (optarg)
                    tcpport = strtol(optarg,NULL,10);
                else
                    tcpport = 0;
                break;
            case 'u':
                if (optarg)
                    udpport = strtol(optarg,NULL,10);
                else
                    udpport = 0;
                break;
            case 'r':
                if (optarg)
                    rawport = strtol(optarg,NULL,10);
                else
                    rawport = 0;
                break;
        }
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check age, size, owner, group and permission property of a file.");

    printf("\n\n");

    print_usage();

    print_help_default();
    printf(" -f, --file=filename\n");
    printf("      The file to test.\n");
    printf(" -w, --warning=time[d|h|m|s]\n");
    printf("      Return warning if the file age exceed this range.\n");
    printf(" -c, --critical=time[d|h|m|s]\n");
    printf("      Return critical if the file age exceed this range.\n");
    printf(" -W=size\n");
    printf("      Return warning if the file size exceed this range.\n");
    printf(" -C=size\n");
    printf("      Return critical if the file size exceed this range.\n");
    printf(" -o, --owner=uanme|uid\n");
    printf("      Return critical if the file don't belong to the user.\n");
    printf(" -g, --group=gname|gid\n");
    printf("      Return critical if the file don't belong to the group.\n");
    printf(" -a, -access=accessstring\n");
    printf("      Return critical if the file permission don't match the accessstring.\n");

    printf("\nAccess String Example:\n");
    printf(" u+r  => Check if file owner can read the file.\n");
    printf(" g=rx => Check if group can read, execute and not write.\n");
    printf(" o-rw => Check if others can't read nor write.\n");

}

/* vim: set ts=4 sw=4 et syn=c.libdns : */
