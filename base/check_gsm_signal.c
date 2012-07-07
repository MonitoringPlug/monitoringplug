/***
 * Monitoring Plugin - check_gsm_signal.c
 **
 *
 * check_gsm_signal - Check GSM Modem signal quality by AT.
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

const char *progname  = "check_gsm_signal";
const char *progdesc  = "Check GSM Modem signal quality by AT.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--serial <DEV>";

/* MP Includes */
#include "mp_common.h"
#include "mp_serial.h"
#include "sms_utils.h"
/* Default Includes */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <error.h>
#include <string.h>

/* Global Vars */
thresholds *dbm_thresholds = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    int i;
    int fd;
    int sq;
    char *cmd;
    char *operator = NULL;
    char *network = NULL;
    char **answer = NULL;
    int answers;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Open Modem Serial Port */
    fd = mp_serial_open(mp_serial_device, mp_serial_speed);

    // Wait until. modem is ready
    for(i = 0; i < 5; i++) {
        if (mobile_at_command(fd, "", NULL, NULL, NULL) == 0)
            break;
        sleep(1);
    }
    if (i == 5) {
        mp_serial_close(fd);
        critical("Modem not ready.");
    }

    // Check for pin
    if (mobile_at_command(fd, "+CPIN", "?", &answer, &answers) != 0) {
        mp_serial_close(fd);
        critical("Checking pin faild.");
    }
    if (answers == 1 && strcmp(answer[0], "SIM PIN") == 0) {
        if (mp_sms_pin) {
            mp_array_free(&answer, &answers);
            mp_asprintf(&cmd,"=\"%s\"", mp_sms_pin);
            mobile_at_command(fd, "+CPIN", cmd, &answer, &answers);
            free(cmd);
            // Recheck pin
            mobile_at_command(fd, "+CPIN", "?", &answer, &answers);
            if (strcmp(answer[0], "READY") != 0) {
                mp_serial_close(fd);
                mp_array_free(&answer, &answers);
                critical("SIM unlock faild. Wrong PIN.");
            }
        } else {
            mp_serial_close(fd);
            mp_array_free(&answer, &answers);
            critical("SIM ask for PIN.");
        }
    } else if (answers == 1 && strcmp(answer[0], "READY") != 0) {
        mp_serial_close(fd);
        mp_array_free(&answer, &answers);
        critical("SIM ask for %s.", answer[0]);
    }
    mp_array_free(&answer, &answers);

    // Check Operator
    if (mobile_at_command(fd, "+COPS", "?", &answer, &answers) == 0) {
        // Offline +COPS: 0
        if (strlen(answer[0]) == 1) {
            mp_serial_close(fd);
            critical("Offline");
        }
        char *ptr;
        int format = 0;
        ptr = answer[0];
        strsep(&ptr, ","); // Mode
        format = (int)strtol(strsep(&ptr, ","), NULL, 10); // Format
        operator = strdup(strsep(&ptr, ","));

        switch (*ptr) {
            case '0':
            case '1':
                network = strdup("GSM");
                break;
            case '2':
                network = "UMTS";
                break;
            case '3':
                network = "EDGE";
                break;
            case '4':
                network = "HSDPA";
                break;
            case '5':
                network = "HSUPA";
                break;
            case '6':
                network = "HSPA";
                break;
            case '7':
                network = "LTE";
                break;
            default:
                network = "5G or newer";
        }
        if (format == 2) {
            // Query operator names
            mp_array_free(&answer, &answers);
            mobile_at_command(fd, "+COPN", NULL, &answer, &answers);
            for (i=0; i < answers; i++) {
                if (strncmp(answer[i], operator, strlen(operator)) == 0) {
                    ptr = answer[i];
                    strsep(&ptr, ",");
                    free(operator);
                    operator = strdup(ptr);
                    break;
                }
            }
        }
        // Remove " "
        if (operator && operator[0] == '"') {
            operator += 1;
            operator[strlen(operator)-1] = '\0';
        }
    }
    mp_array_free(&answer, &answers);

    // Check signal qualiy
    if (mobile_at_command(fd, "+CSQ", NULL, &answer, &answers) == 0) {
        sq = (int)strtol(answer[0], NULL, 10);
        if (sq < 0 || sq > 31) {
            mp_serial_close(fd);
            unknown("Illegal signal quality reading '%s0", answer[0]);
        }
        sq = sq*2 - 113;
        mp_perfdata_int("signal", sq, "dBm", dbm_thresholds);
    } else {
        mp_serial_close(fd);
        unknown("Can't read Signal quality");
    }

    mp_serial_close(fd);

    switch (get_status(sq, dbm_thresholds)) {
        case STATE_OK:
            if (operator)
                ok("%s [%s] %ddBm", operator, network, sq);
            ok("Signal Quality %ddBm", sq);
        case STATE_WARNING:
            if (operator)
                warning("%s [%s] %ddBm", operator, network, sq);
            warning("Signal Quality %ddBm", sq);
        case STATE_CRITICAL:
            if (operator)
                critical("%s [%s] %ddBm", operator, network, sq);
            critical("Signal Quality %ddBm", sq);
    }


    ok("%s [%s] %ddBm", operator, network, sq);
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_SERIAL_LONGOPTS,
        SMS_LONGOPTS,
        MP_LONGOPTS_END
    };

    /* Set default */
    setWarn(&dbm_thresholds, "-90:", 0);
    setCrit(&dbm_thresholds, "-100:", 0);

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"P:w:c:"MP_SERIAL_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_serial(c);
        getopt_wc(c, optarg, &dbm_thresholds);

        switch (c) {
            /* Default opts */
            case 'P':
                mp_sms_pin = optarg;
                break;
        }
    }
    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_serial();
    printf(" -P, --pin=PIN\n");
    printf("      PIN to unlock the SIM.\n");
    print_help_warn("Signal Quality", "-90dBm");
    print_help_crit("Signal Quality", "-100dBm");
}

/* vim: set ts=4 sw=4 et syn=c : */
