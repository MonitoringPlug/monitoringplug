/***
 * Monitoring Plugin - check_snmp_ups.c
 **
 *
 * check_snmp_ups - Check ups battery by snmp.
 *
 * Copyright (C) 2012 Oliver Schonefeld <schonefeld@ids-mannheim.de>
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

const char *progname  = "check_snmp_ups";
const char *progdesc  = "Check status of a UPS conforming to RFC 1628 by SNMP.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Oliver Schonefeld <schonefeld@ids-mannheim.de>";
const char *progusage = "-H <HOST>";

/* MP Includes */
#include "mp_common.h"
#include "snmp_utils.h"
/* Default Includes */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
/* Library Includes */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#define BATTERY_UNKNOWN  1
#define BATTERY_NORMAL   2
#define BATTERY_LOW      3
#define BATTERY_DEPLETED 4

#define OUTPUT_OTHER   1
#define OUTPUT_NONE    2
#define OUTPUT_NORMAL  3
#define OUTPUT_BYPASS  4
#define OUTPUT_BATTERY 5
#define OUTPUT_BOOSTER 6
#define OUTPUT_REDUCER 7

#define DEFAULT_CHARGE_WARNING "@25"   /* in percent */
#define DEFAULT_CHARGE_CRITICAL "@15"  /* in percent */
#define DEFAULT_RUNTIME_WARNING "@10"  /* in minutes */
#define DEFAULT_RUNTIME_CRITICAL "@5"  /* in minutes */

/* Global Vars */
const char  *hostname = NULL;
int         port = 161;
int         extended_status = 0;
thresholds  *threshold_charge = NULL;
thresholds  *threshold_runtime = NULL;

const static char *battery_status_strings[] =
    { "unknown", "normal", "LOW", "DEPLETED" };

const static char *output_source_strings[] =
    { "other", "none", "normal", "BYPASS", "BATTERY", "BOOSTER", "REDUCER" };

inline static const char* battery_status_to_string(int status) {
    return ((status >= BATTERY_UNKNOWN) && (status <= BATTERY_DEPLETED)) ?
        battery_status_strings[status - 1] : "N/A";
}

inline static const char* output_source_to_string(int source) {
    return ((source >= OUTPUT_OTHER) && (source <= OUTPUT_REDUCER)) ?
        output_source_strings[source - 1] : "N/A";
}


int main (int argc, char **argv) {
    /* Local Vars */
    int             state = STATE_OK;
    char            *ups_ident              = NULL;
    long            ups_battery_status      = LONG_MIN;
    long            ups_seconds_on_battery  = LONG_MIN;
    long            ups_remaining_runtime   = LONG_MIN;
    long            ups_remaining_charge    = LONG_MIN;
    long            ups_battery_voltage     = LONG_MIN;
    long            ups_battery_current     = LONG_MIN;
    long            ups_battery_temperature = LONG_MIN;
    long            ups_input_line_bads     = LONG_MIN;
    long            ups_input_lines         = LONG_MIN;
    long            ups_output_source       = LONG_MIN;
    long            ups_output_frequency    = LONG_MIN;
    long            ups_output_lines        = LONG_MIN;
    long            ups_alarms_present      = LONG_MIN;
    char            *output = NULL;
    char            buf[64];
    netsnmp_session *snmp_session;

    mp_snmp_query_cmd snmpcmd[] = {
        {{1,3,6,1,2,1,33,1,1,5,0}, 11, ASN_OCTET_STR,
         (void *)&ups_ident, 0},
        {{1,3,6,1,2,1,33,1,2,1,0}, 11, ASN_INTEGER,
         (void *)&ups_battery_status, sizeof(long int)},
        {{1,3,6,1,2,1,33,1,2,2,0}, 11, ASN_INTEGER,
         (void *)&ups_seconds_on_battery, sizeof(long int)},
        {{1,3,6,1,2,1,33,1,2,3,0}, 11, ASN_INTEGER,
         (void *)&ups_remaining_runtime, sizeof(long int)},
        {{1,3,6,1,2,1,33,1,2,4,0}, 11, ASN_INTEGER,
         (void *)&ups_remaining_charge, sizeof(long int)},
        {{1,3,6,1,2,1,33,1,2,5,0}, 11, ASN_INTEGER,
         (void *)&ups_battery_voltage, sizeof(long int)},     /* 0.1 volts DC */
        {{1,3,6,1,2,1,33,1,2,6,0}, 11, ASN_INTEGER,
         (void *)&ups_battery_current, sizeof(long int)},     /* 0.1 amps DC */
        {{1,3,6,1,2,1,33,1,2,7,0}, 11, ASN_INTEGER,
         (void *)&ups_battery_temperature, sizeof(long int)}, /* deg C */
        {{1,3,6,1,2,1,33,1,3,1,0}, 11, ASN_COUNTER,
         (void *)&ups_input_line_bads, sizeof(long int)},
        {{1,3,6,1,2,1,33,1,3,2,0}, 11, ASN_INTEGER,
         (void *)&ups_input_lines, sizeof(long int)},
        {{1,3,6,1,2,1,33,1,4,1,0}, 11, ASN_INTEGER,
         (void *)&ups_output_source, sizeof(long int)},
        {{1,3,6,1,2,1,33,1,4,2,0}, 11, ASN_INTEGER,
         (void *)&ups_output_frequency, sizeof(long int)}, /* 0.1 RMS */
        {{1,3,6,1,2,1,33,1,4,3,0}, 11, ASN_INTEGER,
         (void *)&ups_output_lines, sizeof(long int)},
        {{1,3,6,1,2,1,33,1,6,1,0}, 11, ASN_GAUGE,
         (void *)&ups_alarms_present, sizeof(long int)},
        {{0}, 0, 0, NULL},
    };

    /* set threshold defaults */
    setWarn(&threshold_charge, DEFAULT_CHARGE_WARNING, NOEXT);
    setCrit(&threshold_charge, DEFAULT_CHARGE_CRITICAL, NOEXT);
    setWarn(&threshold_runtime, DEFAULT_RUNTIME_WARNING, NOEXT);
    setCrit(&threshold_runtime, DEFAULT_RUNTIME_CRITICAL, NOEXT);

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    snmp_session = mp_snmp_init();
    mp_snmp_query(snmp_session, snmpcmd);
    mp_snmp_deinit();

    if (mp_verbose > 1) {
        printf("battery status: %ld\n", ups_battery_status);
        printf("output source: %ld\n", ups_output_source);
        printf("alarms present: %ld\n", ups_alarms_present);
        printf("input lines: %ld\n", ups_input_lines);
        printf("output lines: %ld\n", ups_output_lines);
        printf("input line bads: %ld\n", ups_input_line_bads);
    }

    if (ups_ident) {
        if (*ups_ident != '\0') {
            mp_snprintf((char *)&buf, sizeof(buf), "[%s] ", ups_ident);
            mp_strcat(&output, buf);
        }
        free(ups_ident);
    }

    /* always warning, if on battery */
    if ((ups_seconds_on_battery > 0) ||
        (ups_output_source == OUTPUT_BATTERY)) {
        state = STATE_WARNING;
        if (ups_seconds_on_battery > 0) {
            mp_snprintf((char *) &buf, sizeof(buf),
                        "ON BATTERY (since %d sec%s), ",
                        ups_seconds_on_battery,
                        ((ups_seconds_on_battery != 1) ? "s" : ""));
            mp_strcat(&output, buf);
        } else {
            mp_strcat(&output, "ON BATTERY, ");
        }
    }

    /* check alarms */
    if ((ups_alarms_present != LONG_MIN) && (ups_alarms_present > 0)) {
        if (state != STATE_CRITICAL)
            state = STATE_WARNING;

        if (ups_alarms_present == 1)
            mp_strcat(&output, "UPS REPORTS AN ALARM, ");
        else
            mp_strcat(&output, "UPS REPORTS MULTIPLE ALARMS, ");
    }

    /* check remaining runtime threshold */
    if (ups_remaining_runtime > LONG_MIN) {
        switch (get_status(ups_remaining_runtime, threshold_runtime)) {
        case STATE_CRITICAL:
            state = STATE_CRITICAL;
            mp_strcat(&output, "RUNTIME CRITICAL, ");
            break;
        case STATE_WARNING:
            if (state == STATE_OK)
                state = STATE_WARNING;
            mp_strcat(&output, "RUNTIME WARNING, ");
        } /* switch */
    }

    /* check remaining charge threshold */
    if (ups_remaining_charge > LONG_MIN) {
        switch (get_status(ups_remaining_charge, threshold_charge)) {
        case STATE_CRITICAL:
            state = STATE_CRITICAL;
            mp_strcat(&output, "CHARGE CRITICAL, ");
            break;
        case STATE_WARNING:
            if (state == STATE_OK)
                state = STATE_WARNING;
            mp_strcat(&output, "CHARGE WARNING, ");
            break;
        } /* switch */
    }

    /* check battery status */
    switch (ups_battery_status) {
    case BATTERY_NORMAL:
        break;
    case BATTERY_LOW:
        state = STATE_CRITICAL;
        break;
    case BATTERY_DEPLETED:
        state = STATE_CRITICAL;
        break;
    case BATTERY_UNKNOWN:
        /* FALL-THROUGH */
    default:
        if (state == STATE_OK)
            state = STATE_UNKNOWN;
    } /* switch */

    /* check output source */
    switch (ups_output_source) {
    case OUTPUT_NORMAL:
        /* DO NOTHING */
        break;
    case OUTPUT_OTHER:
        /* FALL-THROUGH */
    case OUTPUT_BYPASS:
        /* FALL-THROUGH */
    case OUTPUT_BOOSTER:
        /* FALL-THROUGH */
    case OUTPUT_REDUCER:
        /* FALL-THROUGH */
    case OUTPUT_BATTERY:
        /* FALL-THROUGH */
        if (state != STATE_CRITICAL)
            state = STATE_WARNING;
        break;
    case OUTPUT_NONE:
        state = STATE_CRITICAL;
        break;
    default:
        if (state == STATE_OK)
            state = STATE_UNKNOWN;
    } /* switch */

    /*
     * status line
     */
    mp_snprintf((char *) &buf, sizeof(buf), "battery: %s",
                battery_status_to_string(ups_battery_status));
    mp_strcat(&output, buf);

    if ((ups_remaining_runtime > LONG_MIN) &&
        (ups_remaining_charge > LONG_MIN)) {
        mp_snprintf((char *) &buf, sizeof(buf),
                    "remaining: %ld min%s [%d%%]",
                    ups_remaining_runtime,
                    ((ups_remaining_runtime != 1) ? "s" : ""),
                    (int) ups_remaining_charge);
        mp_strcat_comma(&output, buf);
    } else if (ups_remaining_runtime > LONG_MIN) {
        mp_snprintf((char *) &buf, sizeof(buf),
                    "remaining: %ld min%s",
                    ups_remaining_runtime,
                    ((ups_remaining_runtime != 1) ? "s" : ""));
        mp_strcat_comma(&output, buf);
    } else if (ups_remaining_charge > LONG_MIN) {
        mp_snprintf((char *) &buf, sizeof(buf),
                    "remaining: %d%%",
                    (int) ups_remaining_charge);
        mp_strcat_comma(&output, buf);
    }

    mp_snprintf((char *) &buf, sizeof(buf), "source: %s",
                output_source_to_string(ups_output_source));
    mp_strcat_comma(&output, buf);

    if (extended_status) {
        if (ups_battery_voltage > LONG_MIN) {
            mp_snprintf((char *) &buf, sizeof(buf),
                        "voltage: %1.0f V",
                        (ups_battery_voltage * 0.1f));
            mp_strcat_comma(&output, buf);
        }
        if (ups_battery_current > LONG_MIN) {
            mp_snprintf((char *) &buf, sizeof(buf),
                        "current: %1.1f A",
                        (ups_battery_current * 0.1f));
            mp_strcat_comma(&output, buf);
        }
        if (ups_battery_temperature > LONG_MIN) {
            mp_snprintf((char *) &buf, sizeof(buf),
                        "temperature: %ld C",
                        ups_battery_temperature);
            mp_strcat_comma(&output, buf);
        }
        if (ups_output_frequency > LONG_MIN) {
            mp_snprintf((char *) &buf, sizeof(buf),
                        "output frequency: %3.1f Hz",
                        (ups_output_frequency * 0.1f));
            mp_strcat_comma(&output, buf);
        }
        if (ups_input_line_bads > LONG_MIN) {
            mp_snprintf((char *) &buf, sizeof(buf),
                        "input line bads: %ld",
                        ups_input_line_bads);
            mp_strcat_comma(&output, buf);
        }
    }

    /*
     * performance data
     */
    if (mp_showperfdata) {
        if (ups_remaining_charge > LONG_MIN) {
            mp_perfdata_int("remaining_charge",
                            ups_remaining_charge,
                            "%", threshold_charge);
        }
        if (ups_remaining_runtime > LONG_MIN) {
            mp_perfdata_int("remaining_runtime",
                            ups_remaining_charge,
                            "minutes", threshold_runtime);
        }
        if (ups_battery_voltage > LONG_MIN) {
            mp_perfdata_float("battery_voltage",
                              ups_battery_voltage * 0.1f,
                              "V", NULL);
        }
        if (ups_battery_current > LONG_MIN) {
            mp_perfdata_float("battery_current",
                              ups_battery_current * 0.1f,
                              "A", NULL);
        }
        if (ups_battery_temperature > LONG_MIN) {
            mp_perfdata_int("battery_temperature",
                            ups_battery_temperature,
                            "C", NULL);
        }
        if (ups_output_frequency > LONG_MIN) {
            mp_perfdata_float("output_frequency",
                              ups_output_frequency * 0.1f,
                              "Hz", NULL);
        }
        if (ups_input_line_bads > LONG_MIN) {
            mp_perfdata_int("input_line_bads",
                            ups_input_line_bads,
                            "", NULL);
        }
    }

    switch (state) {
        case STATE_OK:
            ok("UPS: %s", output);
        case STATE_WARNING:
            warning("UPS: %s", output);
        case STATE_CRITICAL:
            critical("UPS: %s", output);
        default:
            unknown("UPS: %s", output);
    }
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
            MP_LONGOPTS_DEFAULT,
            MP_LONGOPTS_HOST,
            MP_LONGOPTS_PORT,
            {"extended-status", no_argument, NULL, (int) 'e'},
            SNMP_LONGOPTS,
            MP_LONGOPTS_END
    };

    if (argc < 3) {
       print_help();
       exit(STATE_UNKNOWN);
    }

    mp_snmp_version = SNMP_VERSION_1;

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:ew:c:W:Z:"SNMP_OPTSTR,
                      longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_snmp(c);

        switch (c) {
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
            /* Plugin opt */
            case 'e':
                extended_status = 1;
                break;
            case 'w':
                getopt_wc_at(c, optarg, &threshold_charge);
                break;
            case 'c':
                getopt_wc_at(c, optarg, &threshold_charge);
                break;
            case 'W':
                getopt_wc_at('w', optarg, &threshold_runtime);
                break;
            case 'Z':
                getopt_wc_at('c', optarg, &threshold_runtime);
                break;
        }
    }

    if (mp_verbose > 0) {
        print_thresholds("charge", threshold_charge);
        print_thresholds("runtime", threshold_runtime);
    }

    /* sanity check: charge threshold */
    if ((threshold_charge->warning->end < 0) ||
        (threshold_charge->warning->end > 100)) {
        usage("Warning charge threshold must be between 0 and 100.");
    }
    if ((threshold_charge->critical->end < 0) ||
        (threshold_charge->critical->end > 100)) {
        usage("Critical charge threshold must be between 0 and 100.");
    }
    if (threshold_charge->critical->end > threshold_charge->warning->end) {
        usage("Warning charge threshold (%ld) must be larger than "
              "critical charge threshold (%ld).",
              (long) threshold_charge->warning->end,
              (long) threshold_charge->critical->end);
    }

    /* sanity check: runtime threshold */
    if (threshold_runtime->warning->end < 0) {
        usage("Warning runtime threshold must be larher or equal to 0.");
    }
    if (threshold_runtime->critical->end < 0) {
        usage("Critical runtime threshold must be larher or equal to 0.");
    }
    if (threshold_runtime->critical->end > threshold_runtime->warning->end) {
        usage("Warning runtime threshold (%ld) must be larger than "
              "runtime charge threshold (%ld).",
              (long) threshold_runtime->warning->end,
              (long) threshold_runtime->critical->end);
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_revision_snmp();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();

    print_help_warn("remaining charge", DEFAULT_CHARGE_WARNING);
    print_help_crit("remaining charge", DEFAULT_CHARGE_CRITICAL);
    printf(" -W\n");
    printf("      Return warning if remaining runtime exceeds limit. "
           "Defaults to %s\n", DEFAULT_RUNTIME_WARNING);
    printf(" -Z\n");
    printf("      Return critical if remaining runtime exceeds limit. "
           "Defaults to %s\n", DEFAULT_RUNTIME_CRITICAL);
    printf(" -e, --extended-status\n");
    printf("      Print extended status.\n");

    print_help_snmp();
}

/* vim: set ts=4 sw=4 et syn=c : */
