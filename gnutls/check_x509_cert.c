/***
 * Monitoring Plugin - check_x509_cert.c
 **
 *
 * check_x509_cert - Check x509 Certification expiration.
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

const char *progname  = "check_x509_cert";
const char *progdesc  = "Check x509 Certification expiration.";
const char *progvers  = "0.1";
const char *progcopy  = "2014";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--cert <FILE>";

/* MP Includes */
#include "mp_common.h"
#include "mp_utils.h"
#include "mp_net.h"
/* Default Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <glob.h>
/* Library Includes */
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

/* Global Vars */
thresholds *expire_thresholds = NULL;
char **cert_file = NULL;
int cert_files = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    int ret;
    int i;
    unsigned int len;
    gnutls_x509_crt_t cert[1];
    char *subject = NULL;
    size_t subject_len;
    time_t expiration_time, activation_time;


    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    /* Init GnuTLS */
    gnutls_global_init();

    for(i = 0; i < cert_files; i++) {
        if (mp_verbose)
            printf("Cert: %s\n", cert_file[i]);
   
        /* Read the Cert */
        gnutls_datum_t data = { NULL, 0 };
        ret = gnutls_load_file(cert_file[i], &data);
        if (ret != 0) {
            set_critical("Error loading cert file '%s'.\n", cert_file[i]);
            continue;
        }

        /* Load the Cert to a list. */
        len = 1;
        ret = gnutls_x509_crt_list_import(cert, &len, &data,
                GNUTLS_X509_FMT_PEM,
                GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED);
        if (ret < 0) {
            set_critical("%s error: %s", cert_file[i], gnutls_strerror(ret));
            continue;
        };

        /* Read der Cert CN */
        if (subject == NULL) {
            subject = mp_malloc(128);
            subject_len = 128;
        }
        ret = gnutls_x509_crt_get_dn_by_oid(cert[0],
                GNUTLS_OID_X520_COMMON_NAME, 0, 0,
                subject, &subject_len);
        if (ret == GNUTLS_E_SHORT_MEMORY_BUFFER) {
            subject_len+=1;
            subject = mp_realloc(subject, subject_len);
            ret = gnutls_x509_crt_get_dn_by_oid(cert[0],
                    GNUTLS_OID_X520_COMMON_NAME, 0, 0,
                    subject, &subject_len);
        }
        if (ret != 0) {
            set_critical("%s error: %s", cert_file[i], gnutls_strerror(ret));
            continue;
        }
        if (mp_verbose) {
            printf(" * Subject: %s\n", subject);
        }

        /* Check expire time */
        expiration_time = gnutls_x509_crt_get_expiration_time (cert[0]);
        activation_time = gnutls_x509_crt_get_activation_time (cert[0]);

        if (mp_verbose) {
            printf (" * Certificate is valid since: %s", ctime (&activation_time));
            printf (" * Certificate expires: %s", ctime (&expiration_time));
        }

        int days = (int)difftime(expiration_time, time(0))/86400;
        switch (get_status((expiration_time-time(0)), expire_thresholds)) {
            case STATE_OK:
                set_ok(cert_file[i]);
                break;
            case STATE_WARNING:
                set_warning("%s expires in %d day%s", cert_file[i], days, days==1?"":"s");
                break;
            case STATE_CRITICAL:
                set_critical("%s expires in %d day%s", cert_file[i], days, days==1?"":"s");
                break;
        }

        if (activation_time > time(0)) {
            int days = (int)difftime(activation_time, time(0))/86400;
            set_critical("%s activates in %d day%s", cert_file[i], days, days==1?"":"s");
        }
    }

    // Dissconnect
    gnutls_global_deinit ();

    mp_exit("X509");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        // PLUGIN OPTS
        {"cert", required_argument, NULL, (int)'C'},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_END
    };

    mp_threshold_set_warning_time(&expire_thresholds, "30d:");
    mp_threshold_set_critical_time(&expire_thresholds, "10d:");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"w:c:C:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_wc_time_at(c, optarg, &expire_thresholds);

        switch (c) {
            /* Plugin opts */
            case 'C':  {
                glob_t globbuf;
                globbuf.gl_offs = 0;
                glob(optarg, GLOB_BRACE|GLOB_TILDE, NULL, &globbuf);
                int i=0;
                for (i=0; i < globbuf.gl_pathc; i++) {
                    mp_array_push(&cert_file, globbuf.gl_pathv[i], &cert_files);
                }
                break;
            }
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
    printf(" -C, --cert=FILE\n");
    printf("      x509 Cert File to check. (Glob is supported)\n");
    print_help_warn_time("30 days");
    print_help_crit_time("10 days");
}

/* vim: set ts=4 sw=4 et syn=c : */
