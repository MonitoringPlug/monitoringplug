/***
 * Monitoring Plugin - check_smb_share.c
 **
 *
 * check_smb_share - Check smb share connection.
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

const char *progname  = "check_smb_share";
const char *progdesc  = "Check smb share connection.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--url <URL> [--help] [--timeout TIMEOUT]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
/* Library Includes */
#include <libsmbclient.h>

/* Global Vars */
const char *url = NULL;
const char *username = NULL;
const char *password = NULL;
const char *workgroup = NULL;
thresholds *count_threshold = NULL;
thresholds *time_threshold = NULL;

static void get_auth_data_fn(const char * pServer, const char * pShare,
        char * pWorkgroup, int maxLenWorkgroup, char * pUsername,
        int maxLenUsername, char * pPassword, int maxLenPassword) {
    if (mp_verbose >= 1) {
        printf("Auth for: smb://%s/%s - %s\n", pServer, pShare, pWorkgroup);
    }

    if (workgroup)
        strncpy(pWorkgroup, workgroup, maxLenWorkgroup - 1);
    if(username && password) {
        strncpy(pUsername, username, maxLenUsername - 1);
        strncpy(pPassword, password, maxLenPassword - 1);
    }

    return;
}

int main (int argc, char **argv) {
    /* Local Vars */
    SMBCCTX *           context;
    int                 dir;
    struct smbc_dirent  *dirent;
    struct timeval      start_time;
    double              time_delta;

    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Create SMB context
    if (access("/etc/nagios/.smb/smb.conf", R_OK) == 0) {
        setenv("HOME","/etc/nagios",1);
    } else {
        unsetenv("HOME");
    }

    context = smbc_new_context();
    if (!context) {
        critical("Could not allocate new smbc context");
    }
    //setenv("HOME","/etc/nagios",1);

    // Context configuration
    smbc_setFunctionAuthData(context, get_auth_data_fn);
    smbc_setTimeout(context, mp_timeout);
    smbc_setDebug(context, mp_verbose);
    if (username) {
        smbc_setOptionNoAutoAnonymousLogin(context, 1);
    }

    // Init Context
    if (!smbc_init_context(context)) {
        smbc_free_context(context, 0);
        critical("Could not initialize smbc context");
    }

    smbc_set_context(context);

    // Start Timer
    gettimeofday(&start_time, NULL);

    if ((dir = smbc_opendir(url)) < 0) {
        critical("SMB %s - %s", url, strerror(errno));
    }

    if (mp_verbose >= 1) {
        while ((dirent = smbc_readdir(dir)) != NULL) {
            switch(dirent->smbc_type) {
                case SMBC_WORKGROUP:
                    printf(" - [WG] ");
                    break;
                case SMBC_SERVER:
                    printf(" - [Server] ");
                    break;
                case SMBC_FILE_SHARE:
                    printf(" - [File] ");
                    break;
                case SMBC_PRINTER_SHARE:
                    printf(" - [Printer] ");
                    break;
                case SMBC_COMMS_SHARE:
                    printf(" - [Comms] ");
                    break;
                case SMBC_IPC_SHARE:
                    printf(" - [IPC] ");
                    break;
                case SMBC_DIR:
                    printf(" - [DIR] ");
                    break;
                case SMBC_FILE:
                    printf(" - [FILE] ");
                    break;
                case SMBC_LINK:
                    printf(" - [LINK] ");
                    break;
            }
            printf("%s\n", dirent->name);
        }
    }

    smbc_closedir(dir);

    time_delta = mp_time_delta(start_time);

    smbc_free_context(context, 1);

    mp_perfdata_float("time", (float)time_delta, "s", time_threshold);

    switch (get_status(time_delta, time_threshold)) {
        case STATE_OK:
            ok("SMB %s", url);
        case STATE_WARNING:
            warning("SMB %s is slow", url);
        default:
            critical("SMB %s is too slow.", url);
    }
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        // PLUGIN OPTS
        {"username", required_argument, NULL, (int)'U'},
        {"password", required_argument, NULL, (int)'P'},
        {"workgroup", required_argument, NULL, (int)'W'},
        MP_LONGOPTS_TIMEOUT,
        MP_LONGOPTS_END
    };

    /* Set default */
    setWarnTime(&time_threshold, "1s");
    setCritTime(&time_threshold, "2s");

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"u:U:P:W:w:c:H:t:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc(c, optarg, &time_threshold);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Host opt */
            case 'u':
                url = optarg;
                break;
            /* Plugin opts */
            case 'U':
                username = optarg;
                break;
            case 'P':
                password = optarg;
                break;
            case 'W':
                workgroup = optarg;
                break;
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
                break;
        }
    }

    /* Check requirements */
    if (!url)
        usage("URL is mandatory.");
    if (username && !password)
        usage("Username requires password.");

    /* Apply defaults */

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
    printf(" -u, --url=URL\n");
    printf("      Test given smb url.\n");
    printf(" -U, --username=USERNAME\n");
    printf("      Authenticate as USERNAME. (Requires password.)\n");
    printf(" -D, --password=PASSWORD\n");
    printf("      Authenticate with PASSWORD.\n");
    printf(" -W, --workgroup=WORKGROUP\n");
    printf("      Authenticate to WORKGROUP\n");
    print_help_warn_time("1s");
    print_help_crit_time("2s");
}

/* vim: set ts=4 sw=4 et syn=c : */
