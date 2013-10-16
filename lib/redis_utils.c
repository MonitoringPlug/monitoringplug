/***
 * Monitoring Plugin - redis_utils.c
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

#include "mp_common.h"
#include "redis_utils.h"

#include <stdio.h>

void mp_redis_print_reply(redisReply *reply, int ident);

void mp_redis_print_reply(redisReply *reply, int ident) {
    int i = 0;

    printf("< ");
    for (i=0; i<ident; i++)
        printf(" ");

    switch (reply->type) {
        case REDIS_REPLY_STRING:
            printf("'%s'\n", reply->str);
            break;
        case REDIS_REPLY_ARRAY:
            printf("ARRAY\n");
            for (i=0; i<reply->elements; i++)
                mp_redis_print_reply(reply->element[i], ident+1);
            break;
        case REDIS_REPLY_INTEGER:
            printf("%lld\n", reply->integer);
            break;
        case REDIS_REPLY_NIL:
            printf("NIL\n");
            break;
        case REDIS_REPLY_STATUS:
            printf("STATUS: %s\n", reply->str);
            break;
        case REDIS_REPLY_ERROR:
            printf("Error: %s\n", reply->str);
            break;
    }
}

void *mp_redisCommand(redisContext *c, const char *format, ...) {
    va_list ap;
    redisReply *reply = NULL;

    va_start(ap, format);

    if (mp_verbose >= 3) {
        printf("> ");
        vprintf(format, ap);
        printf("\n");
    }

    reply = redisvCommand(c, format, ap);

    va_end(ap);

    if (reply == NULL)
        critical("Error: %s", c->errstr);
    if (reply->type == REDIS_REPLY_ERROR)
        critical("Error: %s", reply->str);

    if (mp_verbose >= 3)
        mp_redis_print_reply(reply, 0);

    return reply;
}



void print_revision_redis(void) {
    printf(" hiredis v%d.%d.%d\n", HIREDIS_MAJOR, HIREDIS_MINOR, HIREDIS_PATCH);
}

/* vim: set ts=4 sw=4 et syn=c : */
