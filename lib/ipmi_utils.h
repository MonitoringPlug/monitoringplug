/***
 * Monitoring Plugin - ipmi_utils.h
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

#ifndef _IPMI_UTILS_H_
#define _IPMI_UTILS_H_

#include "config.h"
#include <getopt.h>

#include <OpenIPMI/ipmiif.h>
#include <OpenIPMI/ipmi_posix.h>

extern int mp_ipmi_entity;
extern int mp_ipmi_readingtype;
extern int mp_ipmi_open;
extern char *mp_ipmi_username;
extern char *mp_ipmi_password;
extern int mp_ipmi_smi;

#define IPMI_OPTSTR "u:p:"
#define IPMI_LONGOPTS {"username", required_argument, NULL, (int)'u'}, \
                      {"password", required_argument, NULL, (int)'p'}, \
                      {"smi", required_argument, NULL, (int)MP_LONGOPT_PRIV0}

struct mp_ipmi_sensor_list {
    /** Sendor */
    ipmi_sensor_t *sensor;
    /** Senor Name */
    char *name;
    /** Sensor value */
    double value;
    /** Sensor states */
    ipmi_states_t *states;
    /** Sendor thresholds */
    thresholds *sensorThresholds;
    /** Next Sensor */
    struct mp_ipmi_sensor_list *next;
};

extern struct mp_ipmi_sensor_list *mp_ipmi_sensors;

/**
 * Global Vars
 */

os_handler_t    *mp_ipmi_hnd;
ipmi_con_t      *mp_ipmi_con;
ipmi_domain_t   *mp_ipmi_dom;

/**
 * Init the OpenIPMI library and return a new handler.
 */
void mp_ipmi_init(void);

/**
 * Cleanup the OpenIPMI library.
 */
void mp_ipmi_deinit();

/**
 * Handle IPMI related command line options.
 * \param[in] c Command line option to handle.
 */
void getopt_ipmi(int c);

/**
 * Print the help for the IPMI related command line options.
 */
void print_help_ipmi(void);

/**
 * Print the OpenIPMI library revision.
 */
void print_revision_ipmi(void);

#endif /* _IPMI_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
