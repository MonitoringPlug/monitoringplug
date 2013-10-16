/***
 * Monitoring Plugin - redis_utils.h
 **
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

#ifndef _REDIS_UTILS_H_
#define _REDIS_UTILS_H_

#include "config.h"
#include <hiredis/hiredis.h>

/**
 * Execute a redis command and do some error detection
 */
void *mp_redisCommand(redisContext *c, const char *format, ...);

/**
 * Print the hiredis revision.
 */
void print_revision_redis(void);

#endif /* _REDIS_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
