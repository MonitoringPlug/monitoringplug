/**
 * Monitoring Plugin - mp_common.h
 **
 *
 * Copyright (C) 2010 Marius Rieder <marius.rieder@durchmesser.ch>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "mp_common.h"

#include <signal.h>:w
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

unsigned int mp_timeout = 10;
unsigned int mp_verbose = 0;

void ok(const char *fmt, ...) {
    va_list ap;
    printf("OK - ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    exit(STATE_OK);
}

void warning(const char *fmt, ...) {
    va_list ap;
    printf("WARNING - ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    exit(STATE_WARNING);
}

void critical(const char *fmt, ...) {
    va_list ap;
    printf("CRITICAL -  ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    exit(STATE_CRITICAL);
}

void unknown(const char *fmt, ...) {
    va_list ap;
    printf("UNKNOWN -  ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    exit(STATE_UNKNOWN);
}

void usage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    print_usage();
    exit(STATE_UNKNOWN);
}

void print_usage (void) {
    printf ("Usage:\n");
    printf (" %s %s\n", progname, progusage);
}

void print_revision (void) {
    printf("%s v%s (%s %s)\n", progname, progvers, PACKAGE_NAME, PACKAGE_VERSION);
}

void print_copyright (void) {
    printf("Copyright (c) %s %s\n", progcopy, progauth);
    printf("Copyright (c) 2010 Monitoring Plugins\n");
}

void timeout_alarm_handler (int signo){
    if (signo == SIGALRM) {
        printf("CRITICAL - Plugin timed out after %d seconds\n", mp_timeout);
        exit(STATE_CRITICAL);
    }
}
