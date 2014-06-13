/***
 * Monitoring Plugin - mp_common.c
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

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned int mp_timeout = 10;
unsigned int mp_verbose = 0;
int mp_state   = -1;
char *mp_out_ok = NULL;
char *mp_out_okonly = NULL;
char *mp_out_warning = NULL;
char *mp_out_critical = NULL;

void ok(const char *fmt, ...) {
    va_list ap;
    printf("OK - ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    if (mp_showperfdata && mp_perfdata) {
       printf("| %s", mp_perfdata);
       free(mp_perfdata);
    }
    printf("\n");
    exit(STATE_OK);
}

void set_ok(const char *fmt, ...) {
    if (mp_state < STATE_OK)
        mp_state = STATE_OK;

    int len=0;
    va_list ap;

    // Estimate len
    va_start(ap, fmt);
    len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    // Get buffer
    if (mp_out_ok) {
        mp_out_ok = mp_realloc(mp_out_ok, strlen(mp_out_ok) + len + 3);
        strcpy(mp_out_ok+strlen(mp_out_ok), ", ");
    } else {
        mp_out_ok = mp_malloc(len);
        *mp_out_ok = '\0';
    }

    // sprintf
    va_start(ap, fmt);
    vsnprintf(mp_out_ok+strlen(mp_out_ok), len+1, fmt, ap);
    va_end(ap);
}

void set_okonly(const char *fmt, ...) {
    if (mp_state > STATE_OK)
        return;

    int len=0;
    va_list ap;

    // Estimate len
    va_start(ap, fmt);
    len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    // Get buffer
    mp_out_okonly = mp_malloc(len);
     *mp_out_okonly = '\0';

    // sprintf
    va_start(ap, fmt);
    vsnprintf(mp_out_okonly, len+1, fmt, ap);
    va_end(ap);
}

void warning(const char *fmt, ...) {
    va_list ap;
    printf("WARNING - ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    if (mp_showperfdata && mp_perfdata) {
       printf(" | %s", mp_perfdata);
       free(mp_perfdata);
    }
    printf("\n");
    exit(STATE_WARNING);
}

void set_warning(const char *fmt, ...) {
    if (mp_state < STATE_WARNING)
        mp_state = STATE_WARNING;

    int len=0;
    va_list ap;

    // Estimate len
    va_start(ap, fmt);
    len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    // Get buffer
    if (mp_out_warning) {
        mp_out_warning = mp_realloc(mp_out_warning, strlen(mp_out_warning) + len + 3);
        strcpy(mp_out_warning+strlen(mp_out_warning), ", ");
    } else {
        mp_out_warning = mp_malloc(len);
        *mp_out_warning = '\0';
    }

    // sprintf
    va_start(ap, fmt);
    vsnprintf(mp_out_warning+strlen(mp_out_warning), len+1, fmt, ap);
    va_end(ap);
}

void critical(const char *fmt, ...) {
    va_list ap;
    printf("CRITICAL - ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    if (mp_showperfdata && mp_perfdata) {
       printf(" | %s", mp_perfdata);
       free(mp_perfdata);
    }
    printf("\n");
    exit(STATE_CRITICAL);
}

void set_critical(const char *fmt, ...) {
    if (mp_state < STATE_CRITICAL)
        mp_state = STATE_CRITICAL;

    int len=0;            
    va_list ap;                

    // Estimate len
    va_start(ap, fmt);         
    len = vsnprintf(NULL, 0, fmt, ap);             
    va_end(ap);       

    // Get buffer   
    if (mp_out_critical) {
        mp_out_critical = mp_realloc(mp_out_critical, strlen(mp_out_critical) + len + 3);
        strcpy(mp_out_critical+strlen(mp_out_critical), ", ");
    } else {
        mp_out_critical = mp_malloc(len);
        *mp_out_critical = '\0';
    }

    // sprintf              
    va_start(ap, fmt);       
    vsnprintf(mp_out_critical+strlen(mp_out_critical), len+1, fmt, ap);
    va_end(ap);   
}

void unknown(const char *fmt, ...) {
    va_list ap;
    printf("UNKNOWN - ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    if (mp_showperfdata && mp_perfdata) {
       printf(" | %s", mp_perfdata);
       free(mp_perfdata);
    }
    printf("\n");
    exit(STATE_UNKNOWN);
}

void mp_exit(const char *fmt, ...) {
    va_list ap;
    switch (mp_state) {
        case -1:
        case STATE_OK:
            mp_state = STATE_OK;
            printf("OK - ");
            break;
        case STATE_WARNING:
            printf("WARNING - ");
            break;
        case STATE_CRITICAL:
            printf("CRITICAL - ");
            break;
    }
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    if (mp_out_critical) {
        printf(" %s", mp_out_critical);
    }
    if (mp_out_warning) {
        if (mp_state > STATE_WARNING)
            printf(" Warning:");
        printf(" %s", mp_out_warning);
    }
    if (mp_out_ok) {
        if (mp_state > STATE_OK)
            printf(" OK:");
        printf(" %s", mp_out_ok);
    }
    if (mp_out_okonly && mp_state == STATE_OK) {
        printf(" %s", mp_out_okonly);
    }
    if (mp_showperfdata && mp_perfdata) {
        printf(" | %s", mp_perfdata);
        free(mp_perfdata);
    }
    printf("\n");
    exit(mp_state);
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
    printf("Copyright (c) 2010-2014 Monitoring Plugins\n");
}

void timeout_alarm_handler(int signo) {
    if (signo == SIGALRM) {
        critical("Plugin timed out after %d seconds\n", mp_timeout);
    }
}

void mp_noneroot_die(void) {
    if (geteuid() != 0) {
        usage("This plugin must be run as root.");
    }
}

/* vim: set ts=4 sw=4 et syn=c : */
