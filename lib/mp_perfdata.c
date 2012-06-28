/***
 * Monitoring Plugin - mp_perfdata.c
 **
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

#include "mp_perfdata.h"
#include "mp_args.h"
#include "mp_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

unsigned int mp_showperfdata = 0;
char *mp_perfdata=NULL;

void mp_perfdata_int(const char *label, long int value, const char *unit,
        thresholds *threshold) {
    mp_perfdata_int2(label, value, unit, threshold, 0, 0, 0, 0);
}

void mp_perfdata_int2(const char *label, long int value, const char *unit,
        thresholds *threshold, int have_min, long int min,
        int have_max, long int max) {
    char *buf;

    if (!mp_showperfdata)
        return;

    if (strpbrk (label, "'= +")) {
        mp_asprintf(&buf, "'%s'=%ld%s;", label, value, unit);
    } else {
        mp_asprintf(&buf, "%s=%ld%s;", label, value, unit);
    }
    mp_strcat_space(&mp_perfdata, buf);
    free(buf);

    if (threshold->warning) {
        buf = str_range(threshold->warning);
        mp_strcat(&mp_perfdata, buf);
        free(buf);
    }
    mp_strcat(&mp_perfdata, ";");

    if (threshold->critical) {
        buf = str_range(threshold->critical);
        mp_strcat(&mp_perfdata, buf);
        free(buf);
    }
    mp_strcat(&mp_perfdata, ";");

    if(have_min) {
        mp_asprintf(&buf, "%ld;", min);
        mp_strcat(&mp_perfdata, buf);
        free(buf);
    }
    mp_strcat(&mp_perfdata, ";");

    if(have_max) {
        mp_asprintf(&buf, "%ld;", max);
        mp_strcat(&mp_perfdata, buf);
        free(buf);
    }

    for( buf = mp_perfdata + strlen(mp_perfdata) - 1; *(buf-1) == ';'; buf--) {
        *buf = '\0';
    }
}

void mp_perfdata_int3(const char *label, long int value, const char *unit,
      int have_warn, long int warn, int have_crit, long int crit,
      int have_min, long int min, int have_max, long int max) {
    thresholds *threshold = NULL;

    if (have_warn || have_crit) {
        threshold = mp_malloc(sizeof(thresholds));
        memset(threshold, 0, sizeof(thresholds));
        if (have_warn) {
            threshold->warning = mp_malloc(sizeof(range));
            memset(threshold->warning, 0, sizeof(range));
            threshold->warning->start_infinity = 1;
            threshold->warning->end = (double) warn;
        }
        if (have_crit) {
            threshold->critical = mp_malloc(sizeof(range));
            memset(threshold->critical, 0, sizeof(range));
            threshold->critical->start_infinity = 1;
            threshold->critical->end = (double) crit;
        }
    }

    mp_perfdata_int2(label, value, unit, threshold,
            have_min, min, have_max, max);

    free_threshold(threshold);
}

void mp_perfdata_float(const char *label, float value, const char *unit,
        thresholds *threshold) {
    mp_perfdata_float2(label, value, unit, threshold, 0, 0, 0, 0);
}

void mp_perfdata_float2(const char *label, float value, const char *unit,
        thresholds *threshold, int have_min, float min,
        int have_max, float max) {
    char *buf;
    int precision = 3;

    if (!mp_showperfdata)
        return;

    if (value >= 9999)
        precision = 0;
    else if (value == 0)
        precision = 0;

    if (strpbrk (label, "'= +")) {
        mp_asprintf(&buf, "'%s'=%.*f%s;", label, precision, value, unit);
    } else {
        mp_asprintf(&buf, "%s=%.*f%s;", label, precision, value, unit);
    }
    mp_strcat_space(&mp_perfdata, buf);
    free(buf);

    if (threshold->warning) {
        buf = str_range(threshold->warning);
        mp_strcat(&mp_perfdata, buf);
        free(buf);
    }
    mp_strcat(&mp_perfdata, ";");

    if (threshold->critical) {
        buf = str_range(threshold->critical);
        mp_strcat(&mp_perfdata, buf);
        free(buf);
    }
    mp_strcat(&mp_perfdata, ";");

    if(have_min) {
        mp_asprintf(&buf, "%.*f;", precision, min);
        mp_strcat(&mp_perfdata, buf);
        free(buf);
    }
    mp_strcat(&mp_perfdata, ";");

    if(have_max) {
        mp_asprintf(&buf, "%.*f;", precision, max);
        mp_strcat(&mp_perfdata, buf);
        free(buf);
    }

    for( buf = mp_perfdata + strlen(mp_perfdata) - 1; *(buf-1) == ';'; buf--) {
        *buf = '\0';
    }
}

void mp_perfdata_float3(const char *label, float value, const char *unit,
      int have_warn, float warn, int have_crit, float crit,
      int have_min, float min, int have_max, float max) {
    thresholds *threshold = NULL;

    if (have_warn || have_crit) {
        threshold = mp_malloc(sizeof(thresholds));
        memset(threshold, 0, sizeof(thresholds));
        if (have_warn) {
            threshold->warning = mp_malloc(sizeof(range));
            memset(threshold->warning, 0, sizeof(range));
            threshold->warning->start_infinity = 1;
            threshold->warning->end = (double) warn;
        }
        if (have_crit) {
            threshold->critical = mp_malloc(sizeof(range));
            memset(threshold->critical, 0, sizeof(range));
            threshold->critical->start_infinity = 1;
            threshold->critical->end = (double) crit;
        }
    }

    mp_perfdata_int2(label, value, unit, threshold,
            have_min, min, have_max, max);

    free_threshold(threshold);
}

/* vim: set ts=4 sw=4 et syn=c : */
