/***
 * Monitoring Plugin - check_file
 **
 *
 * check_file - Check a files property.
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

const char *progname  = "check_file";
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
char *filename = NULL;
char *ownername = NULL;
char *groupname = NULL;
char *accessstring = NULL;
thresholds *age_thresholds = NULL;
thresholds *size_thresholds = NULL;

/* function prototype */
int check_access(mode_t file_stat);

int main (int argc, char **argv) {

    int         status;
    int         age_status = -1;
    int         size_status = -1;
    char        *output = NULL;
    struct stat file_stat;

    if (process_arguments (argc, argv) == 1)
        exit(STATE_CRITICAL);

    if (filename == 0)
        usage("Filename is mandatory.");

    status = lstat(filename, &file_stat);

    if (status != 0)
        critical("Stat '%s' faild.", filename);

    if (mp_verbose > 0) {
        printf("Stat for %s\n", filename);
        printf(" UID:    %d\n", (int) file_stat.st_uid);
        printf(" GID:    %d\n", (int) file_stat.st_gid);
        printf(" MTIME:  %u\n", (unsigned int) file_stat.st_mtime);
        printf(" Size:   %u\n", (unsigned int)(unsigned int)  file_stat.st_size);
    }

    if (age_thresholds != NULL) {
        age_status = get_status((time(0) - file_stat.st_mtime), age_thresholds);

        switch (age_status) {
            case STATE_WARNING:
                output = malloc(sizeof(char) * 12);
                strcat(output, "age warning");
                break;
            case STATE_CRITICAL:
                output = malloc(sizeof(char) * 13);
                strcat(output, "age critical");
                break;
        }
    }

    if (size_thresholds != NULL) {
        size_status = get_status(file_stat.st_size, size_thresholds);

        switch (size_status) {
            case STATE_WARNING:
                if (output != NULL) {
                    output = realloc(output, strlen(output) + sizeof(char) * 15);
                    strcat(output, ", size warning");
                } else {
                    output = malloc(sizeof(char) * 13);
                    strcat(output, "size warning");
                }
                break;
            case STATE_CRITICAL:
                if (output != NULL) {
                    output = realloc(output, strlen(output) + sizeof(char) * 16);
                    strcat(output, ", size critical");
                } else {
                    output = malloc(sizeof(char) * 14);
                    strcat(output, "size critical");
                }
                break;
        }
    }

    status = age_status > size_status ? age_status : size_status;

    if (ownername != NULL) {
        if (is_integer(ownername)) {
            if (file_stat.st_uid != (int) strtol(ownername, NULL, 10)) {
                status = STATE_CRITICAL;
                if (output != NULL) {
                    output = realloc(output, strlen(output) + sizeof(char) * 17);
                    strcat(output, ", owner critical");
                } else {
                    output = malloc(sizeof(char) * 15);
                    strcat(output, "owner critical");
                }
            } else {
                status = status > STATE_OK ? status : STATE_OK;
            }
        } else {
            struct passwd *pwd;
            pwd = getpwnam(ownername);

            if (mp_verbose && pwd)
                printf("Resolv UID: %s => %u\n", ownername, (*pwd).pw_uid);

            if (pwd == NULL || file_stat.st_uid != (*pwd).pw_uid) {
                status = STATE_CRITICAL;
                if (output != NULL) {
                    output = realloc(output, strlen(output) + sizeof(char) * 17);
                    strcat(output, ", owner critical");
                } else {
                    output = malloc(sizeof(char) * 15);
                    strcat(output, "owner critical");
                }
            } else {
                status = status > STATE_OK ? status : STATE_OK;
            }
        }
    }

    if (groupname != 0 ) {
        if (is_integer(groupname)) {
            if (file_stat.st_gid != (int) strtol(groupname, NULL, 10)) {
                status = STATE_CRITICAL;
                if (output != NULL) {
                    output = realloc(output, strlen(output) + sizeof(char) * 17);
                    strcat(output, ", group critical");
                } else {
                    output = malloc(sizeof(char) * 15);
                    strcat(output, "group critical");
                }
            } else {
                status = status > STATE_OK ? status : STATE_OK;
            }
        } else {
            struct group *grp;
            grp = getgrnam(groupname);

            if (mp_verbose && grp)
                printf("Resolv GID: %s => %u\n", groupname, (*grp).gr_gid);

            if (grp == NULL || file_stat.st_gid != (*grp).gr_gid) {
                status = STATE_CRITICAL;
                if (output != NULL) {
                    output = realloc(output, strlen(output) + sizeof(char) * 17);
                    strcat(output, ", group critical");
                } else {
                    output = malloc(sizeof(char) * 15);
                    strcat(output, "group critical");
                }
            } else {
                status = status > STATE_OK ? status : STATE_OK;
            }
        }
    }

    if (accessstring != NULL) {
        if (check_access(file_stat.st_mode) != 0) {
            status = STATE_CRITICAL;
            if (output != NULL) {
                output = realloc(output, strlen(output) + sizeof(char) * 18);
                strcat(output, ", access critical");
            } else {
                output = malloc(sizeof(char) * 16);
                strcat(output, "access critical");
            }
        } else {
            status = status > STATE_OK ? status : STATE_OK;
        }
    }

    switch (status) {
        case STATE_OK:
            ok("%s: Everithing ok.", filename);
        case STATE_WARNING:
            warning("%s: %s", filename, output);
        case STATE_CRITICAL:
            critical("%s: %s", filename, output);
    }



    critical("You should never reach this point.");
}

int check_access(mode_t fmode) {
    char *c;

    int state = 0;
    int subject_mask = 0;
    int mask_function = 0;
    int access_mask = 0;

    for (c = accessstring; c[0] != '\0'; c++) {

        if (state == 1 && c[0] != 'r' && c[0] != 'w' && c[0] != 'x' ) {
            if (mask_function == -1) {
                access_mask &= subject_mask;
                if ((fmode & access_mask) != 0)
                    return -1;
            } else if (mask_function == 0) {
                if ((fmode & subject_mask) != (access_mask &= subject_mask))
                    return -1;
            } else if (mask_function == 1) {
                access_mask &= subject_mask;
                if ((fmode & access_mask) != access_mask)
                    return -1;
            }

            subject_mask = 0;
            mask_function = 0;
            access_mask = 0;

            state = 0;
        }

        if (state == 0) {
            if (c[0] == 'u')
                subject_mask |= S_IRWXU;
            else if (c[0] == 'g')
                subject_mask |= S_IRWXG;
            else if (c[0] == 'o')
                subject_mask |= S_IRWXO;
            else if (c[0] == '-') {
                mask_function = -1;
                state++;
            } else if (c[0] == '+') {
                mask_function = 1;
                state++;
            } else if (c[0] == '=') {
                mask_function = 0;
                state++;
            } else
                unknown("Illegal access string: %s", accessstring);
        }

        if (state == 1) {
            if (c[0] == 'r') {
                access_mask |= S_IRUSR | S_IRGRP | S_IROTH;
            } else if (c[0] == 'w') {
                access_mask |= S_IWUSR | S_IWGRP | S_IWOTH;
            } else if (c[0] == 'x') {
                access_mask |= S_IXUSR | S_IXGRP | S_IXOTH;
            }
        }

    } // for (c = accessstring; c[0] !=...

    if (mask_function == -1) {
        access_mask &= subject_mask;
        if ((fmode & access_mask) != 0)
            return -1;
    } else if (mask_function == 0) {
        if ((fmode & subject_mask) != (access_mask &= subject_mask))
            return -1;
    } else if (mask_function == 1) {
        access_mask &= subject_mask;
        if ((fmode & access_mask) != access_mask)
            return -1;
    }

    return 0;

}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"file", required_argument, NULL, (int)'f'},
        {"owner", required_argument, NULL, (int)'o'},
        {"group", required_argument, NULL, (int)'g'},
        {"access", required_argument, NULL, (int)'a'},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:f:o:g:a:w:c:W:C:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time(c, optarg, &age_thresholds);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            case 'f':
                filename = optarg;
                break;
            case 'o':
                ownername = optarg;
                break;
            case 'g':
                groupname = optarg;
                break;
            case 'a':
                accessstring = optarg;
                break;
            case 'W':
                if (setWarn(&size_thresholds, optarg, BISI) == ERROR)
                    usage("Illegal -W argument '%s'.", optarg);
                break;
            case 'C':
                if (setCrit(&size_thresholds, optarg, BISI) == ERROR)
                    usage("Illegal -C argument '%s'.", optarg);
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
