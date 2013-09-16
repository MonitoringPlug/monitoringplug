/***
 * Monitoring Plugin - check_varnish_backends.c
 **
 *
 * check_varnish_backends - This plugin checks Varnish backend status.
 *
 * Copyright (C) 2013 Marius Rieder <marius.rieder@durchmesser.ch>
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

const char *progname  = "check_varnish_backends";
const char *progdesc  = "This plugin checks Varnish backend status.";
const char *progvers  = "0.1";
const char *progcopy  = "2013";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-n <varnish_name>]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
/* Library Includes */
#include <varnishapi.h>

/* Global Vars */
struct VSM_data *vd;
int short_frame = 5;
thresholds *fail_thresholds = NULL;
thresholds *long_thresholds = NULL;
char *backends_warning = NULL;
char *backends_critical = NULL;

int mp_varnish_stats_cb(void *priv, const struct VSC_point *const pt);

int main (int argc, char **argv) {
    /* Local vars */
    //unsigned int status;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Init VSM Data */
    vd = VSM_New();
    VSC_Setup(vd);

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Open the Varnish SHM */
    if (VSM_Open(vd, mp_verbose)) {
        critical("Varnish: Could not open shared memory for '%s'", VSM_Name(vd));
    }

    /* Reading the stats */
    (void)VSC_Iter(vd, mp_varnish_stats_cb, NULL);

    /* Close SHM */
    VSM_Delete(vd);

    if (backends_critical != NULL)
        critical("Varnish-Backends: %s", backends_critical);
    if (backends_warning != NULL)
        warning("Varnish-Backends: %s", backends_warning);

    ok("Varnish-Backends");
}

int mp_varnish_stats_cb(void *priv, const struct VSC_point *const pt) {
    uint64_t val;
    char *name, *buf;
    int ok = 0;
    int ok_short = 0;
    float ok_perc;
    int i = 0;

    /* Skip all but VBE.*.happy */
    if (strcmp(pt->class, "VBE") != 0)
        return 0;
    if (strcmp(pt->name, "happy") != 0)
        return 0;

    val = *(const volatile uint64_t*)pt->ptr;

    if (mp_verbose > 2) {
        if (strcmp(pt->class, "") != 0)
            printf("%s.", pt->class);
        if (strcmp(pt->ident, "") != 0)
            printf("%s.", pt->ident);
        printf("%s: ", pt->name);
        if (strcmp(pt->fmt, "uint64_t") == 0) {
            printf("%12ju:%c", val, (char)pt->flag);
        }
        printf("\n");
    }

    name = strdup(pt->ident);
    name = strsep(&name, "(");

    /* Count bitmap */
    for (i = 0; i < short_frame; i++) {
        ok_short += (val & 0x1);
        val = val >> 1;
    }
    for (ok = ok_short; i < 64; i++ ) {
        ok += (val & 0x1);
        val = val >> 1;
    }

    /* Perfdata */
    mp_perfdata_int2(name, ok_short, "", fail_thresholds, 1, 0, 1, short_frame);
    mp_asprintf(&buf, "%s_long" , name);
    mp_perfdata_int2(buf, ok, "", NULL, 1, 0, 1, 64);
    free(buf);

    ok_perc = ((float)ok_short*100)/((float)short_frame);

    switch(get_status(ok_perc, fail_thresholds)) {
        case STATE_WARNING:
            mp_asprintf(&buf, "%s (%d/%d)" , name, ok_short, short_frame);
            mp_strcat_comma(&backends_warning, buf);
            free(buf);
            break;
        case STATE_CRITICAL:
            mp_asprintf(&buf, "%s (%d/%d)" , name, ok_short, short_frame);
            mp_strcat_comma(&backends_critical, buf);
            free(buf);
            break;
    }

    free(name);

    return 0;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;
    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        {"short", required_argument, 0, 's'},
        MP_LONGOPTS_END
    };

    setWarn(&fail_thresholds, "90:", NOEXT);
    setCrit(&fail_thresholds, "80:", NOEXT);

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"s:w:c:"VSC_ARGS, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc(c, optarg, &fail_thresholds);

        switch (c) {
            case 'n':
                VSC_Arg(vd, c, optarg);
                break;
            case 's':
                short_frame = (int)strtol(optarg, NULL, 10);
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

    printf(" -s, --short\n");
    printf("      Short range for evaluation. (Default to: 5)\n");
    print_help_warn("OK in %", "90:");
    print_help_crit("OK in %", "80:");
}

/* vim: set ts=4 sw=4 et syn=c : */
