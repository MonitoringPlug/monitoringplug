/***
 * Monitoring Plugin - ipmi_utils.c
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

#include "mp_common.h"
#include "ipmi_utils.h"

#include <stdio.h>
#include <string.h>
#include <OpenIPMI/ipmi_smi.h>
#include <OpenIPMI/ipmi_lan.h>
#include <OpenIPMI/ipmi_auth.h>

/**
 * Privat function
 */
static void mp_ipmi_log(os_handler_t *hnd, const char *format,
        enum ipmi_log_type_e log_type, va_list ap);
void mp_ipmi_setup_done(ipmi_domain_t *domain, int err, unsigned int conn_num,
        unsigned int port_num, int still_connected, void *user_data);
void mp_ipmi_domain_up(ipmi_domain_t *domain, void *user_data);
static void mp_ipmi_entity_change(enum ipmi_update_e op,
        ipmi_domain_t* domain, ipmi_entity_t *entity, void *user_data);
static void mp_ipmi_sensor_change(enum ipmi_update_e op, ipmi_entity_t *ent,
        ipmi_sensor_t *sensor, void *user_data);
static void mp_ipmi_sensor_read_handler (ipmi_sensor_t *sensor, int err,
        enum ipmi_value_present_e value_present,
        unsigned int __attribute__((unused)) raw_value,
        double value, ipmi_states_t __attribute__((unused)) *states,
        void *user_data);
static void mp_ipmi_sensor_thresholds_handler(ipmi_sensor_t *sensor, int err,
        ipmi_thresholds_t *th, void *user_data);
static void mp_ipmi_sensor_states_handler(ipmi_sensor_t *sensor, int err,
        ipmi_states_t *states, void *user_data);

/**
 * Global Varables
 */
int mp_ipmi_init_done = 0;
int mp_ipmi_entity = IPMI_ENTITY_ID_UNSPECIFIED;
int mp_ipmi_readingtype = 0;
int mp_ipmi_open = IPMI_OPEN_OPTION_SDRS;
struct mp_ipmi_sensor_list *mp_ipmi_sensors = NULL;
const char *mp_ipmi_hostname = NULL;
const char *mp_ipmi_port = "623";
const char *mp_ipmi_username = NULL;
const char *mp_ipmi_password = NULL;
#if OS_LINUX
int mp_ipmi_smi=-1;
#endif

void mp_ipmi_init(void) {
    int rv = 1;

    /* OS handler allocated first. */
    mp_ipmi_hnd = ipmi_posix_setup_os_handler();

    if (!mp_ipmi_hnd)
        unknown("Can't allocate OpenIPMI OS Handler.");

    /* Override the default log handler. */
    mp_ipmi_hnd->set_log_handler(mp_ipmi_hnd, mp_ipmi_log);

    /* Initialize the OpenIPMI library. */
    if (mp_verbose > 1)
        printf("Init OpenIPMI OS Handler.\n");
    ipmi_init(mp_ipmi_hnd);

    if (mp_verbose > 1)
        printf("Connect OpenIPMI.\n");
    if (mp_ipmi_hostname) {
        rv = ipmi_ip_setup_con((char * const*)&mp_ipmi_hostname,
                (char * const*)&mp_ipmi_port, 1,
                IPMI_AUTHTYPE_DEFAULT, IPMI_PRIVILEGE_ADMIN,
                (void *)mp_ipmi_username, strlen(mp_ipmi_username),
                (void *)mp_ipmi_password, strlen(mp_ipmi_password),
                mp_ipmi_hnd, NULL, &mp_ipmi_con);
#if OS_LINUX
    } else {
        rv = ipmi_smi_setup_con(0, mp_ipmi_hnd, NULL, &mp_ipmi_con);
#endif
    }

    if (rv)
        unknown("Can't setup OpenIPMI connection: %s", strerror(rv));

    ipmi_open_option_t open_option[2];
    memset (open_option, 0, sizeof (open_option));
    open_option[0].option = IPMI_OPEN_OPTION_ALL;
    open_option[0].ival = 0;
    open_option[1].option = mp_ipmi_open;
    open_option[1].ival = 1;

    if (mp_verbose > 1)
        printf("Open OpenIPMI Domain.\n");
    rv = ipmi_open_domain(progname, &mp_ipmi_con, 1,
            mp_ipmi_setup_done, NULL,
            mp_ipmi_domain_up, NULL,
            open_option, sizeof (open_option) / sizeof (open_option[0]), NULL);

    if (rv)
        unknown("Can't open OpenIPMI domain: %s", strerror(rv));

    while(!mp_ipmi_init_done) {
        mp_ipmi_hnd->perform_one_op(mp_ipmi_hnd, NULL);
    }

    return;
}

void mp_ipmi_deinit(void) {
    if (mp_verbose > 1)
        printf("Free OpenIPMI OS Handler.\n");
    mp_ipmi_hnd->free_os_handler(mp_ipmi_hnd);
}

void getopt_ipmi(int c) {
    switch ( c ) {
         /* Hostname opt */
        case 'H':
#if OS_LINUX
            if (mp_ipmi_smi != -1)
                usage("--hostname and --smi are exclusive options.");
#endif
            getopt_host_ip(optarg, &mp_ipmi_hostname);
            break;
        /* Port opt */
        case 'P':
            getopt_port(optarg, NULL);
            mp_ipmi_port = optarg;
            break;
        case 'u':
            if (strlen(optarg) > 16)
                usage("IPMI max username leng is 16.");
            mp_ipmi_username = optarg;
            break;
        case 'p':
            if (strlen(optarg) > 16)
                usage("IPMI max username leng is 16.");
            mp_ipmi_password = optarg;
            break;
#if OS_LINUX
        case MP_LONGOPT_PRIV0:
            if (mp_ipmi_hostname)
                usage("--hostname and --smi are exclusive options.");
            mp_ipmi_smi = (int) strtol(optarg, NULL, 10);
            break;
#endif
    }
}

void print_help_ipmi(void) {
    print_help_host();
    print_help_port("Default to 623");
    printf(" -u, --username=USERNAME\n");
    printf("      User name for IPMI lan connect.\n");
    printf(" -p, --password=PASSWORD\n");
    printf("      Authentication password.\n");
#if OS_LINUX
    printf("     --smi=INDEX\n");
    printf("      SMI to connect to. (Default to 0)\n");
#endif
}

void print_revision_ipmi(void) {
    printf(" OpenIPMI v%s\n", ipmi_openipmi_version());
}

// Private Functions
static void mp_ipmi_log(os_handler_t *hnd, const char *format,
                enum ipmi_log_type_e log_type, va_list ap) {
    int nl = 1;

    switch (log_type) {
        case IPMI_LOG_INFO:
            if (mp_verbose < 3) return;
            printf("OpenIPMI Info: ");
            break;
        case IPMI_LOG_WARNING:
            if (mp_verbose < 2) return;
            printf("OpenIPMI Warn: ");
            break;
        case IPMI_LOG_SEVERE:
            printf("OpenIPMI Serv: ");
            break;
        case IPMI_LOG_FATAL:
            printf("OpenIPMI Fatl: ");
            break;
        case IPMI_LOG_ERR_INFO:
            printf("OpenIPMI Einf: ");
            break;
        case IPMI_LOG_DEBUG_START:
            nl = 0;
            /* no break | FALLTHROUGH */
        case IPMI_LOG_DEBUG:
            if (mp_verbose < 4) return;
            printf("OpenIPMI Debg: ");
            break;
        case IPMI_LOG_DEBUG_CONT:
            nl = 0;
            /* no break | FALLTHROUGH */
        case IPMI_LOG_DEBUG_END:
            break;
    }

    vprintf(format, ap);

    if (nl)
        printf("\n");
}

void mp_ipmi_setup_done(ipmi_domain_t *domain, int err, unsigned int conn_num,
                unsigned int port_num, int still_connected, void *user_data) {
    int rv;

    /* Register a callback function entity_change. When a new entities
       is created, entity_change is called */
    rv = ipmi_domain_add_entity_update_handler(domain, mp_ipmi_entity_change, domain);
    if (rv)
        unknown("ipmi_domain_add_entity_update_handler return error: %", rv);
}

void mp_ipmi_domain_up(ipmi_domain_t *domain, void *user_data) {
    struct mp_ipmi_sensor_list *s, *n;

    if (mp_verbose > 1)
        printf("OpenIPMI Domain Up.\n");
     mp_ipmi_dom = domain;

     // Clean empty sensors
     for (s=mp_ipmi_sensors; s; s=s->next) {
         while (s->next && !s->next->name) {
             n = s->next->next;
             free_threshold(s->next->sensorThresholds);
             free(s->next);
             s->next = n;
         }
     }

     mp_ipmi_init_done = 1;
}

static void mp_ipmi_entity_change(enum ipmi_update_e op, ipmi_domain_t *domain,
        ipmi_entity_t *entity, void *user_data) {
    int id, rv;

    id = ipmi_entity_get_entity_id(entity);

    if (mp_verbose > 3)
        printf("[Entity Change: %s %s]\n", ipmi_update_e_string(op),
                ipmi_get_entity_id_string(id));

    if (op != IPMI_ADDED)
        return;

    if (mp_ipmi_entity != IPMI_ENTITY_ID_UNSPECIFIED && mp_ipmi_entity != id)
        return;

    rv = ipmi_entity_add_sensor_update_handler(entity, mp_ipmi_sensor_change,
            entity);

    if (rv)
        unknown("ipmi_entity_set_sensor_update_handler: 0x%x", rv);
}

static void mp_ipmi_sensor_change(enum ipmi_update_e op, ipmi_entity_t *ent,
        ipmi_sensor_t *sensor, void *user_data) {
    char *name;
    int len;
    int type;
    struct mp_ipmi_sensor_list *s;

    len = ipmi_sensor_get_id_length(sensor);
    name = mp_malloc(len+1);
    memset(name, 0, len+1);
    ipmi_sensor_get_id(sensor, name, len);

    if (mp_verbose > 3)
        printf("[Sensor Change %s '%s' '%s:%s']\n", ipmi_update_e_string(op),
                ipmi_sensor_get_event_reading_type_string(sensor),
                ipmi_get_entity_id_string(ipmi_entity_get_entity_id(ent)),
                name);

    if (op != IPMI_ADDED || ipmi_sensor_get_is_readable(sensor) == 0) {
        free(name);
        return;
    }

    s = mp_malloc(sizeof(struct mp_ipmi_sensor_list));
    memset(s, 0, sizeof(struct mp_ipmi_sensor_list));
    s->name = name;
    s->sensor = sensor;
    s->next = mp_ipmi_sensors;
    mp_ipmi_sensors = s;

    type = ipmi_sensor_get_event_reading_type(sensor);

    if (mp_ipmi_readingtype && type != mp_ipmi_readingtype)
        return;

    if (type == IPMI_EVENT_READING_TYPE_THRESHOLD) {
        ipmi_sensor_get_reading(sensor, mp_ipmi_sensor_read_handler, s);
        ipmi_sensor_get_thresholds(sensor, mp_ipmi_sensor_thresholds_handler, s);
    } else {
        ipmi_sensor_get_states(sensor, mp_ipmi_sensor_states_handler, s);
    }
}

static void mp_ipmi_sensor_read_handler (ipmi_sensor_t *sensor, int err,
        enum ipmi_value_present_e value_present,
        unsigned int __attribute__((unused)) raw_value,
        double value, ipmi_states_t __attribute__((unused)) *states,
        void *user_data) {
    struct mp_ipmi_sensor_list *s;
    s = (struct mp_ipmi_sensor_list *) user_data;

    if (mp_verbose > 3) {
        if (value_present == IPMI_NO_VALUES_PRESENT)
            printf("[Sensor Read None %s]\n", s->name);
        else if (value_present == IPMI_RAW_VALUE_PRESENT)
            printf("[Sensor Read Raw %s %d]\n", s->name, raw_value);
        else if (value_present == IPMI_BOTH_VALUES_PRESENT)
            printf("[Sensor Read %s %f]\n", s->name, value);
    }

    if (value_present == IPMI_NO_VALUES_PRESENT
            || !ipmi_is_sensor_scanning_enabled(states)
            || ipmi_is_initial_update_in_progress(states)) {
        free(s->name);
        s->name = NULL;
        return;
    }

    s->value = value;
}

static void mp_ipmi_sensor_thresholds_handler(ipmi_sensor_t *sensor, int err,
        ipmi_thresholds_t *th, void *user_data) {
    struct mp_ipmi_sensor_list *s;
    double val;
    int rv;

    s = (struct mp_ipmi_sensor_list *) user_data;

    if (mp_verbose > 3)
        printf("[Sensor Thresholds %s]\n", s->name);

    if (err != 0)
        return;

    setWarn(&s->sensorThresholds, "~:", -1);
    setCrit(&s->sensorThresholds, "~:", -1);

    rv = ipmi_threshold_get(th, IPMI_LOWER_CRITICAL, &val);
    if (rv == 0) {
        s->sensorThresholds->critical->start = val;
        s->sensorThresholds->critical->start_infinity = 0;
    }

    rv = ipmi_threshold_get(th, IPMI_LOWER_NON_CRITICAL, &val);
    if (rv == 0) {
        s->sensorThresholds->warning->start = val;
        s->sensorThresholds->warning->start_infinity = 0;
    }

    rv = ipmi_threshold_get(th, IPMI_UPPER_CRITICAL, &val);
    if (rv == 0) {
        s->sensorThresholds->critical->end = val;
        s->sensorThresholds->critical->end_infinity = 0;
    }

    rv = ipmi_threshold_get(th, IPMI_UPPER_NON_CRITICAL, &val);
    if (rv == 0) {
        s->sensorThresholds->warning->end = val;
        s->sensorThresholds->warning->end_infinity = 0;
    }

    if (mp_verbose > 3)
        print_thresholds(s->name, s->sensorThresholds);
}

static void mp_ipmi_sensor_states_handler(ipmi_sensor_t *sensor, int err,
        ipmi_states_t *states, void *user_data) {
    struct mp_ipmi_sensor_list *s;

    s = (struct mp_ipmi_sensor_list *) user_data;

    if (mp_verbose > 3) {
        printf("[Sensor States %s]\n", s->name);
        int i;
        for (i=0; i< 16; i++)
            if (ipmi_is_state_set(states, i))
                printf(" %s\n", ipmi_sensor_reading_name_string(sensor, i));
    }

    if (err != 0)
        return;

    s->states = states;
}

/* vim: set ts=4 sw=4 et syn=c : */
