/***
 * Monitoring Plugin - check_mem.c
 **
 *
 * check_mem - Check memory usaged.
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

const char *progname  = "check_mem";
const char *progdesc  = "Check memory usaged.";
const char *progvers  = "0.1";
const char *progcopy  = "2010";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-w <warning usage>] [-c <critical usage>]";

/* MP Includes */
#include "mp_common.h"
/* Default Includes */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/* Global Vars */
thresholds *usage_thresholds = NULL;

/* Function prototype */
float readValue(char *str);

int main (int argc, char **argv) {
    /* Local Vars */
    FILE    *meminfo;
    char    line[64];
    char    *line_ptr;
    char    *key;
    float       mem_total = 0, mem_free = 0;
    float       slab = 0;
    float       swap_cached = 0, swap_total = 0, swap_free = 0;
    float       page_tables = 0;
    float       buffers = 0;
    float       cached = 0;
    float       apps, swap, used;
    float       usedp;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Read /proc/meminfo
    meminfo = fopen("/proc/meminfo", "r");
    if (meminfo == NULL)
        unknown("Can't read /proc/meminfo");

    while (fgets(line, 64, meminfo) != NULL) {
        line_ptr = line;

        key = strsep(&line_ptr, " ");

        if (strcmp("MemTotal:", key) == 0) {
            mem_total = readValue(line_ptr);
        } else if (strcmp("MemFree:", key) == 0) {
            mem_free = readValue(line_ptr);
        } else if (strcmp("Slab:", key) == 0) {
            slab = readValue(line_ptr);
        } else if (strcmp("SwapCached:", key) == 0) {
            swap_cached = readValue(line_ptr);
        } else if (strcmp("SwapTotal:", key) == 0) {
            swap_total = readValue(line_ptr);
        } else if (strcmp("SwapFree:", key) == 0) {
            swap_free= readValue(line_ptr);
        } else if (strcmp("PageTables:", key) == 0) {
            page_tables = readValue(line_ptr);
        } else if (strcmp("Buffers:", key) == 0) {
            buffers = readValue(line_ptr);
        } else if (strcmp("Cached:", key) == 0) {
            cached = readValue(line_ptr);
        }
    }

    // Calculations
    apps = mem_total - mem_free - buffers - cached - slab - page_tables
        - swap_cached;
    swap = swap_total - swap_free;
    used = mem_total - mem_free - buffers - cached;

    usedp = (float)(used*100)/(float)mem_total;

    mp_perfdata_float("memtotal", mem_total, "", NULL);
    mp_perfdata_float2("slab", slab, "", NULL, 1, 0, 1, mem_total);
    mp_perfdata_float2("swapcached", swap_cached, "", NULL, 1, 0, 1, mem_total);
    mp_perfdata_float2("pagetables", page_tables, "", NULL, 1, 0, 1, mem_total);
    mp_perfdata_float2("apps", apps, "", NULL, 1, 0, 1, mem_total);
    mp_perfdata_float2("memfree", mem_free, "", NULL, 1, 0, 1, mem_total);
    mp_perfdata_float2("buffers", buffers, "", NULL, 1, 0, 1, mem_total);
    mp_perfdata_float2("cached", cached, "", NULL, 1, 0, 1, mem_total);
    mp_perfdata_float2("swap", swap, "", NULL, 1, 0, 1, mem_total);

    switch (get_status(usedp, usage_thresholds)) {
        case STATE_OK:
            ok("Memory - %2.2f%% (%s of %s) used", usedp, mp_human_size(used), mp_human_size(mem_total));
        case STATE_WARNING:
            warning("Memory - %2.2f%% (%s of %s) used", usedp, mp_human_size(used), mp_human_size(mem_total));
        case STATE_CRITICAL:
            critical("Memory - %2.2f%% (%s of %s) used", usedp, mp_human_size(used), mp_human_size(mem_total));
    }

    unknown("Memory - %2.2f%% (%s of %s) used", usedp, mp_human_size(used), mp_human_size(mem_total));
}

float readValue(char *str) {
    float value;
    char *unit;

    while (isspace(str[0])) {
        str++;
    }

    value = (float)strtod(str, &unit);

    if (unit && !isspace(unit[1])) {
        switch(unit[1]) {
            case 'g':
                value *= 1024;
            case 'm':
                value *= 1024;
            case 'k':
                value *= 1024;
        }
    }

    return value;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_WC,
        MP_LONGOPTS_END,
    };

    /* Set default */
    setWarn(&usage_thresholds, "90", 0);
    setCrit(&usage_thresholds, "95", 0);

    while (1) {
        c = getopt_long(argc, argv, MP_OPTSTR_DEFAULT"c:w:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc(c, optarg, &usage_thresholds);

    }

    /* Check requirements */

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
    print_help_warn("Memory Usage", "90%");
    print_help_crit("Memory Usage", "95%");
}

/* vim: set ts=4 sw=4 et syn=c : */
