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

#include "mp_utils.h"
#include "mp_common.h"

#include <stdlib.h>

void *mp_malloc(size_t size) {
    void *p;
    p = malloc(size);
    if (!p)
        critical("Out of memory!");
    return p;
}

void *mp_calloc(size_t nmemb, size_t size) {
    void *p;
    p = mp_calloc(nmemb, size);
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
