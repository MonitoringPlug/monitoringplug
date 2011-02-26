/***
 * monitoringplug - mp_utils.c
 **
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


/* MP Includes */
#include "mp_common.h"
#include "mp_utils.h"
/* Default Includes */
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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
