/***
 * monitoringplug - check_ssl_cert.c
 **
 *
 * Copyright (C) 2011 Marius Rieder <marius.rieder@durchmesser.ch>
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
 * $Id$
 */

const char *progname  = "check_ssl_cert";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[--help] [--timeout TIMEOUT]";

/* MP Includes */
#include "mp_common.h"
#include "mp_utils.h"
#include "mp_net.h"
/* Default Includes */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
/* Library Includes */
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

/* Global Vars */
thresholds *expire_thresholds = NULL;
const char *hostname = NULL;
int port = 0;
int ipv = AF_UNSPEC;

int main (int argc, char **argv) {
    /* Local Vars */
    int socket;
    int ret;
    int status;
    char *out;
    char *buf;
    size_t len;
    const char *err;
    time_t expire;
    gnutls_session_t session;
    gnutls_certificate_credentials_t xcred;


    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Connect to Server
    socket = mp_connect(hostname, port, ipv, SOCK_STREAM);

    // GNUTLS init
    gnutls_global_init ();
    gnutls_certificate_allocate_credentials (&xcred);
    gnutls_init (&session, GNUTLS_CLIENT);
    gnutls_session_set_ptr (session, (void *) hostname);
    ret = gnutls_priority_set_direct(session, "PERFORMANCE", &err);
    gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);
    
    gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) socket);

    // SSL Handshake
    ret = gnutls_handshake (session);
    if (ret < 0) {
        mp_disconnect(socket);
        gnutls_deinit (session);
        gnutls_certificate_free_credentials (xcred);
        gnutls_global_deinit ();
        unknown("SSL Handshake failed");
    } else if (mp_verbose >= 1) {
        printf ("- Handshake was completed\n");
    }
    
    expire = gnutls_certificate_expiration_time_peers(session);

    gnutls_x509_crt_t cert;
    const gnutls_datum_t *cert_list;
    unsigned int cert_list_size;

    if (gnutls_x509_crt_init (&cert) < 0) {
        gnutls_deinit(session);
        gnutls_certificate_free_credentials(xcred);
        gnutls_global_deinit();
        critical("error in initialization\n");
    }

    cert_list = gnutls_certificate_get_peers(session, &cert_list_size);
    if (cert_list == NULL) {
        gnutls_deinit(session);
        gnutls_certificate_free_credentials(xcred);
        gnutls_global_deinit();
        critical("No certificate was found!");
    }

    if (gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER) < 0) {
        gnutls_deinit(session);
        gnutls_certificate_free_credentials(xcred);
        gnutls_global_deinit();
        critical("error parsing certificate\n");
    }

    len = 200;
    buf = mp_malloc(len);
    gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_COMMON_NAME, 0, 0, buf, &len);

    out = mp_malloc(len + 16);
    mp_sprintf(out, "Cert: %s expires", buf);

    uint32_t now = time(0);
    status = get_status((expire-now), expire_thresholds);

    struct tm *tmp;
    tmp = localtime(&expire);
    strftime(buf, 200, "%F", tmp);

    mp_strcat_space(&out, buf);

    // Dissconnect
    mp_disconnect(socket);
    gnutls_deinit (session);
    gnutls_certificate_free_credentials (xcred);
    gnutls_global_deinit ();

    switch (status) {
        case STATE_OK:
            ok(out);
        case STATE_WARNING:
            warning(out);
        case STATE_CRITICAL:
            critical(out);
    }

    critical("You should never reach this point.");
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        MP_LONGOPTS_PORT,
        // PLUGIN OPTS
        MP_LONGOPTS_WC,
        MP_LONGOPTS_END
    };

    setWarnTime(&expire_thresholds, "30d:");
    setCritTime(&expire_thresholds, "10d:");

    while (1) {
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"H:P:46w:c:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_46(c, &ipv);
        getopt_wc_time(c, optarg, &expire_thresholds);

        switch (c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
            /* Plugin opts */
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
        }
    }

    /* Check requirements */
    if (!hostname || !port)
        usage("Hostname and port mandatory.");

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: check_ssl_cert");

    printf("\n\n");

    print_usage();

    print_help_default();
#ifdef USE_IPV6
    print_help_46();
#endif //USE_IPV6
    //printf(" -f, --file=filename\n");
    //printf("      The file to test.\n");
    printf(" -c, --trusted-ca=[NAME:]FILE\n");
    printf("      File to read trust-CAs from.\n");
    print_help_warn_time("30 days");
    print_help_crit_time("10 days");
}

/* vim: set ts=4 sw=4 et syn=c : */
