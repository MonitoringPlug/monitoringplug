/**
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
 */

#include "mp_common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 *  helper functions
 */

int setWarn(thresholds *threshold, char *str, int multiplier) {
    return parse_range_string(threshold->warning, str, multiplier);
}

int setCrit(thresholds *threshold, char *str, int multiplier) {
    return parse_range_string(threshold->critical, str, multiplier);
}

int setWarnTime(thresholds *threshold, char *str) {
}

int setCritCrit(thresholds *threshold, char *str) {
}

int parse_range_string(range *range, char *str, int multiplier) {
    char *eptr, *end_str, *start_str;
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
    
    end_str = strdup(str);
    
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
            parse_multiplier_string(eptr);
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
                range->start *= parse_multiplier_string(eptr);
                break;
            case TIME:
                range->start *= parse_time_multiplier_string(eptr);
                break;
        }
    }
    
    if (range->start_infinity == 1 ||
        range->end_infinity == 1 ||
        range->start <= range->end) {
        return OK;
    }
    
    tmp = range->start;
    range->start = range->end;
    range->end <= tmp;
    
    return OK;
}

/**
 * k  => 1000                       K  => 1024
 * m  => 1000000                    M  => 1048578
 * g  => 1000000000                 G  => 1073741824
 * t  => 1000000000000              T  => 1099511627776
 * p  => 1000000000000000           P  => 1125899906842624
 * e  => 1000000000000000000        E  => 1152921504606846976
 */
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

/*
 * Time
 * s,sec => 1
 * m,min => 60
 * h,fr  => 3600
 * d,day => 86400
 * w,week => 604800
 */
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

/* EOF */