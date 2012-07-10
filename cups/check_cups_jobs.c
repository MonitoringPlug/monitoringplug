/***
 * Monitoring Plugin - check_cups_jobs.c
 **
 *
 * check_cups_jobs - Check CUPS job count and age.
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

const char *progname  = "check_cups_jobs";
const char *progdesc  = "Check CUPS job count and age.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "-H <HOSTANME> [--help] [--timeout TIMEOUT]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
/* Library Includes */
#include <cups/cups.h>

/* Global Vars */
const char *hostname = NULL;
char **printer = NULL;
int printers = 0;
int summerize = 0;
thresholds *count_threshold = NULL;
thresholds *time_threshold = NULL;


/* Function prototype */

int main (int argc, char **argv) {
    /* Local Vars */
    int         i, j;
    cups_dest_t *dests;
    int         dest_num;
    cups_job_t *jobs;
    int jobs_num = 0;
    int jobs_total = 0;
    int state = STATE_OK;
    int lstate;
    char *warn = NULL;
    char *crit = NULL;
    char buf[65];
    time_t now;

    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Set hostname
    if (hostname) {
        cupsSetServer(hostname);
    }

    // Get printer List
    if (printers == 0) {
        dest_num = cupsGetDests(&dests);
        mp_perfdata_int("printers", dest_num, "", NULL);
    } else {
        dest_num = printers;
        dests = mp_malloc(sizeof(cups_dest_t)*printers);
        for(i=0; i< printers; i++) {
            dests[i] = *cupsGetNamedDest(CUPS_HTTP_DEFAULT, printer[i], NULL);
        }
    }

    for(i=0; i < dest_num; i++) {
        if (mp_verbose > 0) {
            printf(" * %s, %s\n", dests[i].name, dests[i].instance);
            if (mp_verbose > 2) {
                for(j=0; j < dests[i].num_options; j++) {
                    printf("   - %s => %s\n", dests[i].options[j].name, dests[i].options[j].value);
                }
            }
        }

        jobs_num = cupsGetJobs(&jobs, dests[i].name, 0, CUPS_WHICHJOBS_ACTIVE);
        jobs_total += jobs_num;

        if (summerize == 0) {
            lstate = get_status((double)jobs_num, count_threshold);
            state = lstate > state ? lstate : state;
            if (lstate == STATE_WARNING) {
                mp_snprintf(buf, 64, "%s: %d", dests[i].name, jobs_num);
                mp_strcat_comma(&warn, buf);
            } else if (lstate == STATE_CRITICAL) {
                mp_snprintf(buf, 64, "%s: %d", dests[i].name, jobs_num);
                mp_strcat_comma(&crit, buf);
            }
        }

        now = time(NULL);

        if (jobs_num > 0 && time_threshold) {
            for(j=0; j < jobs_num; j++) {
                if (mp_verbose > 1) {
                    printf("   = ID:%d '%s' (%ds)\n", jobs[j].id, jobs[j].title, (int)(now-jobs[j].creation_time));
                }
                lstate = get_status((double)(now-jobs[j].creation_time), time_threshold);
                state = lstate > state ? lstate : state;
                if (lstate == STATE_WARNING) {
                    mp_snprintf(buf, 64, "%d@%s %ds", jobs[j].id, dests[i].name, (int)(now-jobs[j].creation_time));
                    mp_strcat_comma(&warn, buf);
                } else if (lstate == STATE_CRITICAL) {
                    mp_snprintf(buf, 64, "%d@%s %ds", jobs[j].id, dests[i].name, (int)(now-jobs[j].creation_time));
                    mp_strcat_comma(&crit, buf);
                }
            }
        }

        cupsFreeJobs(jobs_num, jobs);
    }

    if (summerize == 1) {
        lstate = get_status((double)jobs_num, count_threshold);
        state = lstate > state ? lstate : state;
        if (lstate == STATE_WARNING) {
            mp_snprintf(buf, 64, "Total Jobs: %d", jobs_total);
            mp_strcat_comma(&warn, buf);
        } else if (lstate == STATE_CRITICAL) {
            mp_snprintf(buf, 64, "Total Jobs: %d", jobs_total);
            mp_strcat_comma(&crit, buf);
        }
        mp_perfdata_int("jobs", jobs_total, "", count_threshold);
    }

    cupsFreeDests(dest_num, dests);

    if (state == STATE_WARNING) {
        warning("CUPS: %s", warn);
    } else if (state == STATE_CRITICAL) {
        if(warn) {
            critical("CUPS: %s (Warn: %s)", crit, warn);
        }
        critical("CUPS: %s", crit);
    }
    ok("CUPS Jobs");

}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        // PLUGIN OPTS
        {"printer", required_argument, NULL, (int)'P'},
        {"summarize", no_argument, NULL, (int)'s'},
        MP_LONGOPTS_END
    };

    /* Set default */
    setWarnTime(&time_threshold, "5m");
    setCritTime(&time_threshold, "10m");

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"P:sw:c:W:C:H:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc(c, optarg, &count_threshold);

        switch (c) {
            /* Host opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Plugin opts */
            case 'P':
                mp_array_push(&printer, optarg, &printers);
                break;
            case 's':
                summerize = 1;
                break;
            case 'W':
                if (setWarnTime(&time_threshold, optarg) == ERROR)
                    usage("Illegal -c warning '%s'.", optarg);
                break;
            case 'C':
                if (setCritTime(&time_threshold, optarg) == ERROR)
                    usage("Illegal -c warning '%s'.", optarg);
        }
    }

    /* Check requirements */

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
    print_help_host();
    printf(" -P, --printer=PRINTER\n");
    printf("      Check device named PRINTER.\n");
    printf(" -s, --summarize\n");
    printf("      Summerize job count over all printers.\n");
    print_help_warn("job count", "none");
    print_help_crit("job count", "none");
    printf(" -W, --timewarning=time[d|h|m|s]\n");
    printf("      Return warning if job processing time exceeds value. Default to 5m\n");
    printf(" -C, --timecritical=time[d|h|m|s]\n");
    printf("      Return critical if job processing time exceeds value. Default to 10m\n");
}

/* vim: set ts=4 sw=4 et syn=c : */
