/***
 * Monitoring Plugin - mp_args.c
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 *  helper functions
 */

int setWarn(thresholds **threshold, const char *str, int multiplier) {
    if(*threshold == NULL) {
        *threshold = (thresholds *) malloc(sizeof(thresholds));
        (*threshold)->warning = NULL;
        (*threshold)->critical = NULL;
    }
    if((*threshold)->warning == NULL)
        (*threshold)->warning = (range *) malloc(sizeof(range));

    return parse_range_string((*threshold)->warning, str, multiplier);
}

int setCrit(thresholds **threshold, const char *str, int multiplier) {
    if(*threshold == NULL) {
        *threshold = (thresholds *) malloc(sizeof(thresholds));
        (*threshold)->warning = NULL;
        (*threshold)->critical = NULL;
    }
    if((*threshold)->critical == NULL)
        (*threshold)->critical = (range *) malloc(sizeof(range));

    return parse_range_string((*threshold)->critical, str, multiplier);
}

int setWarnTime(thresholds **threshold, const char *str) {
    if(*threshold == NULL) {
        *threshold = (thresholds *) malloc(sizeof(thresholds));
        (*threshold)->warning = NULL;
        (*threshold)->critical = NULL;
    }
    if((*threshold)->warning == NULL)
        (*threshold)->warning = (range *) malloc(sizeof(range));

    return parse_range_string((*threshold)->warning, str, TIME);
}

int setCritTime(thresholds **threshold, const char *str) {
    if(*threshold == NULL) {
        *threshold = (thresholds *) malloc(sizeof(thresholds));
        (*threshold)->warning = NULL;
        (*threshold)->critical = NULL;
    }
    if((*threshold)->critical == NULL)
        (*threshold)->critical = (range *) malloc(sizeof(range));

    return parse_range_string((*threshold)->critical, str, TIME);
}

void free_threshold(thresholds *threshold) {
    if(threshold != NULL) {
        if(threshold->critical != NULL)
            free(threshold->critical);
        if(threshold->warning != NULL)
            free(threshold->warning);
    }
}

int parse_range_string(range *range, const char *str, int multiplier) {
    char *e, *eptr, *end_str, *start_str;
    double tmp;

    /* Set defaults */
    range->start = 0;
    range->start_infinity = 0;
    range->end = 0;
    range->end_infinity = 0;
    range->alert_on = OUTSIDE;

    if (str[0] == '@') {
        range->alert_on = INSIDE;
        str++;
    }

    e = end_str = strdup(str);

    start_str = strsep(&end_str, ":");
    if (end_str == NULL) {
        end_str = start_str;
        start_str = NULL;
    }

    if (start_str != NULL) {
        if (start_str[0] == '~') {
            range->start_infinity = 1;
        } else {
            range->start = strtod(start_str, &eptr);
            if (str == eptr)
                return ERROR;
            switch(multiplier) {
                case BISI:
                    range->start *= parse_multiplier_string(eptr);
                    break;
                case TIME:
                    range->start *= parse_time_multiplier_string(eptr);
                    break;
            }
        }
    }

    if (end_str[0] == '~' || strcmp(end_str, "") == 0) {
        range->end_infinity = 1;
    } else {
        range->end = strtod(end_str, &eptr);
        if (end_str == eptr)
            return ERROR;
        switch(multiplier) {
            case BISI:
                range->end *= parse_multiplier_string(eptr);
                break;
            case TIME:
                range->end *= parse_time_multiplier_string(eptr);
                break;
        }
    }

    if (range->start_infinity == 1 ||
        range->end_infinity == 1 ||
        range->start <= range->end) {
        free(e);
        return OK;
    }

    tmp = range->start;
    range->start = range->end;
    range->end = tmp;

    free(e);

    return OK;
}

double parse_multiplier_string(char *str) {

    switch (str[0]) {
        case 'k':
                return 1000;
        case 'K':
                return 1024;
        case 'm':
                return 1000000;
        case 'M':
                return 1048578;
        case 'g':
                return 1000000000;
        case 'G':
                return 1073741824;
        case 't':
                return 1000000000000LL;
        case 'T':
                return 1099511627776LL;
        case 'p':
                return 1000000000000000LL;
        case 'P':
                return 1125899906842624LL;
        case 'e':
                return 1000000000000000000LL;
        case 'E':
                return 1152921504606846976LL;
    }
    return 1;
}

double parse_time_multiplier_string(char *str) {

    switch (str[0]) {
        /* Time */
        case 'm':
            return 60;
        case 'h':
            return 3600;
        case 'd':
            return 86400;
        case 'w':
            return 604800;
    }
    return 1;
}

int check_range(double value, range *my_range) {
    if (value < my_range->start && my_range->start_infinity == 0)
        return (my_range->alert_on == OUTSIDE);

    if (value > my_range->end && my_range->end_infinity == 0)
        return (my_range->alert_on == OUTSIDE);

    return my_range->alert_on == INSIDE;
}

int get_status(double value, thresholds *my_thresholds) {
    if(my_thresholds == NULL)
        return STATE_OK;
	if (my_thresholds->critical != NULL) {
		if (check_range(value, my_thresholds->critical) == TRUE) {
			return STATE_CRITICAL;
		}
	}
	if (my_thresholds->warning != NULL) {
		if (check_range(value, my_thresholds->warning) == TRUE) {
			return STATE_WARNING;
		}
	}
	return STATE_OK;
}

void print_thresholds(const char *threshold_name, thresholds *my_threshold) {
	printf("%s - ", threshold_name);
	if (! my_threshold) {
		printf("Threshold not set");
	} else {
		if (my_threshold->warning) {
			printf("Warning: start=%g end=%g; ", my_threshold->warning->start, my_threshold->warning->end);
		} else {
			printf("Warning not set; ");
		}
		if (my_threshold->critical) {
			printf("Critical: start=%g end=%g", my_threshold->critical->start, my_threshold->critical->end);
		} else {
			printf("Critical not set");
		}
	}
	printf("\n");
}

void print_help_default(void) {
    printf("\n\
Options:\n\
 -h, --help\n\
      Print detailed help screen.\n\
 -V, --version\n\
      Print version information.\n\
 -v, --verbose\n\
      Show details for command-line debugging.\n");
}

void print_help_perf(void) {
    printf("\
 --perfdata\n\
      Print perfdata for check.\n");
}

void print_help_timeout(void) {
	printf("\
 -t, --timeout=INTEGER\n\
      Seconds before  check timesout.\n");
}

void print_help_host(void) {
	printf("\
 -H, --hostname=ADDRESS\n\
      Host name or IP Address.\n");
}

void print_help_port(const char *def) {
	printf("\
 -P, --port=PORT\n\
      Port number to use. Default to %s\n", def);
}

void print_help_warn_time(const char *def) {
	printf("\
 -w, --warning=time[d|h|m|s]\n\
      Return warning if elapsed time exceeds value. Default to %s\n", def);
}

void print_help_crit_time(const char *def) {
	printf("\
 -c, --critical=time[d|h|m|s]\n\
      Return critical if elapsed time exceeds value. Default to %s\n", def);
}

void print_help_warn(const char *limit, const char *def) {
	printf("\
 -w, --warning=LIMIT\n\
      Return warning if %s exceeds limit. Default to %s\n", limit, def);
}

void print_help_crit(const char *limit, const char *def) {
	printf("\
 -c, --critical=LIMIT\n\
      Return critical if %s exceeds limit. Default to %s\n", limit, def);
}

void print_help_46(void) {
    printf("\
 -4, --ipv4\n\
      Use IPv4 to check.\n\
 -6, --ipv6\n\
      Use IPv6 to check.\n");
}

void getopt_default(int c) {
    switch (c) {
        case 'h':
            print_help();
            exit(0);
        case 'V':
            print_revision();
            exit (0);
        case 'v':
            mp_verbose++;
            break;
    }
}

void getopt_perf(int c) {
    switch (c) {
        case ARG_PERFDATA:
            mp_showperfdata = 1;
            break;
    }
}

void getopt_timeout(int c, const char *optarg) {
    if (c == 't')
        mp_timeout = atoi (optarg);
}

void getopt_host(int c, const char *optarg, const char **hostname) {
    if (c == 'H') {
        if (!is_hostname(optarg) && !is_hostaddr(optarg))
            usage("Illegal -H argument '%s'.", optarg);
        *hostname = optarg;
    }
}

void getopt_host_ip(int c, const char *optarg, const char **hostname) {
    if (c == 'H') {
        if (!is_hostaddr(optarg))
            usage("Illegal -H argument '%s'.", optarg);
        *hostname = optarg;
    }
}

void getopt_port(int c, const char *optarg, int *port) {
    if (c == 'P') {
        if (!is_integer(optarg))
            usage("Illegal port number '%s'.", optarg);
        *port = (int) strtol(optarg, NULL, 10);
    }
}

void getopt_wc(int c, const char *optarg, thresholds **threshold) {
    if (c == 'w') {
        if (setWarn(threshold, optarg, BISI) == ERROR)
            usage("Illegal -c warning '%s'.", optarg);
    } else if (c == 'c') {
        if (setCrit(threshold, optarg, BISI) == ERROR) \
                usage("Illegal -c warning '%s'.", optarg);
    }
}

void getopt_wc_time(int c, const char *optarg, thresholds **threshold) {
    if (c == 'w') {
        if (setWarnTime(threshold, optarg) == ERROR)
            usage("Illegal -c warning '%s'.", optarg);
    } else if (c == 'c') {
        if (setCritTime(threshold, optarg) == ERROR) \
                usage("Illegal -c warning '%s'.", optarg);
    }
}

void getopt_46(int c, int *ipv4, int *ipv6) {
    if (c == '4') {
        *ipv4 = 2;
        if(*ipv6 != 2)
            *ipv4 = 0;
    } else if (c == '6') {
        *ipv6 = 2;
        if(*ipv4 != 2)
            *ipv6 = 0;
    }
}

/* vim: set ts=4 sw=4 et : */
