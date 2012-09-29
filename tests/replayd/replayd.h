/***
 * Monitoring Plugin - replayd.h
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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

typedef struct replay_entry_s {
    char                    *name;
    netsnmp_variable_list   *vars;
    struct replay_entry_s   *next;
} replay_entry;


typedef struct type_atoi_s {
    char *a;
    int   i;
} type_atoi;

int ra_type_atoi(const char *type);
netsnmp_variable_list *parse_reply(char *name);
int replay_handler(netsnmp_mib_handler *handler,
        netsnmp_handler_registration *reginfo,
        netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests);

/* vim: set ts=4 sw=4 et syn=c : */
