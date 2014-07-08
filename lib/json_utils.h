/***
 * Monitoring Plugin - json_utils.h
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

#ifndef _JSON_UTILS_H_
#define _JSON_UTILS_H_

#include <json.h>

/**
 * json_object_object_get(_ex) wrapper to support more json-c versions
 */
int mp_json_object_object_get(struct json_object* jso, const char *key, struct json_object **value);

/**
 * Print the json revision.
 */
void print_revision_json(void);

#endif /* _JSON_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
