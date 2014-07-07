/***
 * Monitoring Plugin - json_utils.c
 **
 *
 * Copyright (C) 2014 Marius Rieder <marius.rieder@durchmesser.ch>
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

#include "mp_common.h"
#include "json_utils.h"

#include <stdio.h>

#include <json.h>

json_bool mp_json_object_object_get(struct json_object* jso, const char *key, struct json_object **value) {
#if JSON_C_VERSION_NUM < (10 << 8)
    *value = json_object_object_get(jso, key);
    if *value ! == null
        return TRUE
    else
        return FALSE
#else
    return json_object_object_get_ex(jso, key, value);
#endif
}

void print_revision_json(void) {
    printf(" json-c v%s\n", json_c_version());
}

/* vim: set ts=4 sw=4 et syn=c : */
