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
char **backend = NULL;
int backends = 0;
struct VSM_data *vd;
int range_len = 5;
thresholds *fail_thresholds = NULL;
thresholds *long_thresholds = NULL;
char *backends_warning = NULL;
char *backends_critical = NULL;

int mp_varnish_stats_cb(void *priv, const struct VSC_point *const pt);

int main (int argc, char **argv) {
    /* Local vars */
    int i;

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

    /* Check for unmatched backends */
    if (backends > 0) {
        for(i=0; i < backends; i++) {
            if (backend[i] != NULL && backend[i][strlen(backend[i])-1] != '*') {
                char *buf;
                mp_asprintf(&buf, "%s (not found)" , backend[i]);
                mp_strcat_comma(&backends_critical, buf);
                free(buf);
            }
        }
    }

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
    int ok_range = 0;
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

    /* Check if backend should be checked */
    if (backends > 0) {
        for(i=0; i < backends; i++) {
            if (mp_strmatch(name, backend[i]) == 0)
                continue;
            break;
        }
        if (i >= backends)
            return 0;
        /* Set to NULL as allready matched */
        if (backend[i][strlen(backend[i])-1] != '*')
            backend[i] = NULL;
    }

    /* Count bitmap */
    for (i = 0; i < range_len; i++) {
        ok_range += (val & 0x1);
        val = val >> 1;
    }
    for (ok = ok_range; i < 64; i++ ) {
        ok += (val & 0x1);
        val = val >> 1;
    }

    /* Perfdata */
    mp_perfdata_int2(name, ok_range, "", fail_thresholds, 1, 0, 1, range_len);
    mp_asprintf(&buf, "%s_long" , name);
    mp_perfdata_int2(buf, ok, "", NULL, 1, 0, 1, 64);
    free(buf);

    ok_perc = ((float)ok_range*100)/((float)range_len);

    switch(get_status(ok_perc, fail_thresholds)) {
        case STATE_WARNING:
            mp_asprintf(&buf, "%s (%d/%d)" , name, ok_range, range_len);
            mp_strcat_comma(&backends_warning, buf);
            free(buf);
            break;
        case STATE_CRITICAL:
            mp_asprintf(&buf, "%s (%d/%d)" , name, ok_range, range_len);
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
        {"range", required_argument, 0, 'r'},
        {"backend", required_argument, 0, 'b'},
        MP_LONGOPTS_END
    };

    setWarn(&fail_thresholds, "90:", NOEXT);
    setCrit(&fail_thresholds, "80:", NOEXT);

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"r:b:w:c:"VSC_ARGS, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc(c, optarg, &fail_thresholds);

        switch (c) {
            /* Plugin opts */
            case 'n':
                VSC_Arg(vd, c, optarg);
                break;
            case 'r':
                range_len = (int)strtol(optarg, NULL, 10);
                break;
            case 'b':
                mp_array_push(&backend, optarg, &backends);
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

    printf(" -r, --range=RANGE\n");
    printf("      Range of backend checks to evaluate. (Default to: 5)\n");
    printf(" -b, --backend\n");
    printf("      Backends to test.\n");
    print_help_warn("OK in %", "90:");
    print_help_crit("OK in %", "80:");
}

/* vim: set ts=4 sw=4 et syn=c : */
