/***
 * Monitoring Plugin - mp_utils.c
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


/* MP Includes */
#include "mp_common.h"
#include "mp_utils.h"
/* Default Includes */
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

int mp_sprintf(char *s, const char *format, ...) {
    int len=0;
    va_list ap;
    va_start(ap, format);
    len = vsprintf(s, format, ap);
    va_end(ap);
    if (len < 0)
        critical("sprintf faild!");
    return len;
}

int mp_snprintf(char *s, size_t n, const char *format, ...) {
    int len=0;
    va_list ap;
    va_start(ap, format);
    len = vsnprintf(s, n, format, ap);
    va_end(ap);
    if (len < 0 || len >= n)
        critical("snprintf faild!");
    return len;
}

int mp_asprintf(char **retp, const char *format, ...) {
    int len=0;
    va_list ap;
    char *buf;

    // Estimate len
    va_start(ap, format);
    len = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    // Get buffer
    buf = mp_malloc(len + 1);

    // sprintf
    va_start(ap, format);
    vsnprintf(buf, len+1, format, ap);
    va_end(ap);

    *retp = buf;
    return len;
}

void *mp_malloc(size_t size) {
    void *p;
    p = malloc(size);
    if (!p)
        critical("Out of memory!");
    return p;
}

void *mp_calloc(size_t nmemb, size_t size) {
    void *p;
    p = calloc(nmemb, size);
    if (!p)
        critical("Out of memory!");
    return p;
}

void *mp_realloc(void *ptr, size_t size) {
    void *p;
    p = realloc(ptr, size);
    if (!p) {
        free(ptr);
        critical("Out of memory!");
    }
    return p;
}

void mp_strcat(char **target, char *source) {
    if(source == NULL) {
        return;
    } else if(*target == NULL) {
        *target = strdup(source);
    } else {
        *target = mp_realloc(*target, strlen(*target) + strlen(source) + 1);
        strcat(*target, source);
    }
}

void mp_strcat_space(char **target, char *source) {
    if(source == NULL) {
        return;
    } else if(*target == NULL) {
        *target = strdup(source);
    } else {
        *target = mp_realloc(*target, strlen(*target) + strlen(source) + 2);
        strcat(*target, " ");
        strcat(*target, source);
    }
}

void mp_strcat_comma(char **target, char *source) {
    if(source == NULL) {
        return;
    } else if(*target == NULL) {
        *target = strdup(source);
    } else {
        *target = mp_realloc(*target, strlen(*target) + strlen(source) + 3);
        strcat(*target, ", ");
        strcat(*target, source);
    }
}

void mp_array_push(char ***array, char *obj, int *num) {
    while(obj != NULL) {
        *array = realloc(*array, sizeof(char*)*((*num)+1));
        (*array)[*num] = strsep(&obj, ",");
        (*num)++;
    }
}

double mp_time_delta(struct timeval time_start) {
   struct timeval time_end;

   gettimeofday(&time_end, NULL);
   return ((double)(time_end.tv_sec - time_start.tv_sec) +
	 (double)(time_end.tv_usec - time_start.tv_usec) / (double)1000000);
}

char *mp_human_size(float size) {
    char *out;
    int exp;

    for(exp = 0; exp < 5 && size > 1024; exp++) {
        size /= 1024;
    }

    out = mp_malloc(14);
    sprintf(out, "%.2f ", size);

    switch (exp) {
        case 1:
            strcat(out, "KiB");
            break;
        case 2:
            strcat(out, "MiB");
            break;
        case 3:
            strcat(out, "GiB");
            break;
        case 4:
            strcat(out, "TiB");
            break;
    }

    return out;
}

/* vim: set ts=4 sw=4 et syn=c : */
