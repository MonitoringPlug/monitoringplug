/***
 * Monitoring Plugin - notify_sms.c
 **
 *
 * check_nrped - Check if run inside of nrped.
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

const char *progname  = "notify_sms";
const char *progdesc  = "Check if run inside of nrped.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--number <NUMBER> --msg <MSG>";

/* MP Includes */
#include "mp_common.h"
#include "mp_serial.h"
#include "sms_utils.h"
/* Default Includes */
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <error.h>
#include <string.h>

/* Global Vars */
char *number = NULL;
char *msg = NULL;

int main (int argc, char **argv) {
    /* Local Vars */
    int fd;
    char *cmd;
    char *reply;
    char *pdu;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    fd = mp_serial_open(mp_serial_device, mp_serial_speed);

    mobile_at_command(fd, "", NULL, NULL, NULL);

    char **an = NULL;
    int ans = 0;

    mobile_at_command(fd, "+COPN", NULL, &an, NULL);

    int i;
    for(i=0; i<ans; i++) {
        printf(" * %s\n", an[i]);
    }

    mp_serial_close(fd);
    ok("END");

    // Wait until. modem is ready
    for(i = 0; i < 5; i++) {
        reply = mp_serial_reply(fd, "AT");

        if (strcmp(reply, "OK") == 0) {
            break;
        }
        sleep(1);
    }
    if (i == 5) {
        mp_serial_close(fd);
        critical("Modem not ready.");
    }

    // Check for pin
    reply = mp_serial_reply(fd, "AT+CPIN?");
    if (strcmp(reply, "+CPIN: SIM PIN") == 0) {
        if (mp_sms_pin) {
            mp_asprintf(&cmd, "AT+CPIN=\"%s\"", mp_sms_pin);
            reply = mp_serial_reply(fd, cmd);
            free(reply);
            free(cmd);
            // Recheck pin
            reply = mp_serial_reply(fd, "AT+CPIN?");
            if (strcmp(reply, "+CPIN: READY") != 0) {
                mp_serial_close(fd);
                critical("SIM unlock faild. Wrong PIN.");
            }
        } else {
            mp_serial_close(fd);
            critical("SIM ask for PIN.");
        }
    } else if (strcmp(reply, "+CPIN: SIM PUK") == 0) {
        mp_serial_close(fd);
        critical("SIM ask for PUK.");
    }
    sleep(1);
    printf("--------------\n");
    while(mp_serial_readline(fd, reply, 63) > 0) {
    }
    printf("--------------\n");

    reply = mp_serial_reply(fd, "AT+CMGF=0");
/*
    pdu = sms_encode_pdu(NULL, number, msg);
    printf("PDU: %s\n", pdu);

    mp_asprintf(&cmd, "AT+CMGS=%d", strlen(pdu)/2);
    mp_serial_write(fd, cmd);

    char *ptr;
    ptr = reply;

    read(fd, ptr, 2);
    printf(":%s\n", reply);

    write(fd, pdu, strlen(pdu));
    ptr = reply;

    read(fd, ptr, strlen(pdu));

    printf(":%s\n", reply);

    cmd[0] = '\x1A';
    cmd[1] = '\0';
    write(fd, cmd, 2);

    for (i=0; i<10; i++) {
        mp_serial_readline(fd, reply, 63);
        printf(":%s\n", reply);
    }
*/
    mp_serial_close(fd);

    ok("END");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_SERIAL_LONGOPTS,
        {"msg", required_argument, NULL, (int)'m'},
        {"number", required_argument, NULL, (int)'n'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"t:m:n:"MP_SERIAL_OPTSTR, longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_serial(c);
        switch (c) {
            MP_GETOPTS_DEFAULT
            /* Plugin opts */
            case 'm':
                msg = optarg;
                break;
            case 'n':
                number = optarg;
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
}

/* vim: set ts=4 sw=4 et syn=c : */
