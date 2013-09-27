/***
 * Monitoring Plugin - check_ssl_cert.c
 **
 *
 * check_ssl_cert - Check X509 Certification expiration over ssl.
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

const char *progname  = "check_ssl_cert";
const char *progdesc  = "Check X509 Certification expiration over ssl.";
const char *progvers  = "0.1";
const char *progcopy  = "2011";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "--host <HOSTNAME> --port <PORT>";

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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
/* Library Includes */
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#define MP_LONGOPT_STARTTLS MP_LONGOPT_PRIV0
#define MP_LONGOPT_NOSNI MP_LONGOPT_PRIV1

/* Global Vars */
thresholds *expire_thresholds = NULL;
const char *hostname = NULL;
const char *servername = NULL;
const char *starttls = NULL;
int sni = 1;
int port = 0;
int ipv = AF_UNSPEC;
char **ca_file = NULL;
int ca_files = 0;

int main (int argc, char **argv) {
    /* Local Vars */
    int socket;
    int ret;
    int status;
    unsigned int cstatus;
    int i;
    char *untrusted = NULL;
    char *out;
    char *buf;
    size_t len;
    const char *err;
    time_t expire;
    gnutls_session_t session;
    gnutls_certificate_credentials_t xcred;
    gnutls_x509_crt_t cert;
    const gnutls_datum_t *cert_list;
    unsigned int cert_list_size;


    /* Set signal handling and alarm */
    if (signal (SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap failed!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments failed!");

    /* Start plugin timeout */
    alarm(mp_timeout);

    // Connect to Server
    socket = mp_connect(hostname, port, ipv, SOCK_STREAM);

    // StartTLS handling
    if (starttls) {
        if (strcmp(starttls, "smtp") == 0) {
            char *line;
            int has_starttls = 0;

            // Wait for 220
            line = mp_recv_line(socket);
            if (strncmp(line, "220 ", 4) != 0) {
                mp_disconnect(socket);
                unknown("Don't looks like smtp: %s", line);
            }
            free(line);

            // Send EHLO
            line = mp_malloc(128);
            mp_sprintf(line, "EHLO ");
            gethostname(line+5, 122);
            line[strlen(line)+1] = '\0';
            line[strlen(line)] = '\n';
            send(socket, line, strlen(line), 0);
            if (mp_verbose > 3)
                printf("> %s", line);

            // Read EHLO reply
            do {
                free(line);
                line = mp_recv_line(socket);
                if (strncmp(line, "250", 3) != 0) {
                    mp_disconnect(socket);
                    critical("EHLO failed: %s", line);
                }
                if (strncmp(line+4, "STARTTLS", 8) == 0)
                    has_starttls = 1;
            } while (line && (strncmp(line, "250 ", 4) != 0));
            free(line);

            if (has_starttls == 0)
                critical("SMTP Server do not offer STARTTLS");

            // Ask for STARTTLS
            send(socket, "STARTTLS\n", 9, 0);
            if (mp_verbose > 3)
                printf("> STARTTLS\n");

            // Check reply
            line = mp_recv_line(socket);
            if (strncmp(line, "220 ", 4) != 0) {
                mp_disconnect(socket);
                unknown("STARTTLS Error: %s", line);
            }
        } else if (strcmp(starttls, "pop") == 0) {
            char *line;

            // Wait for banner
            line = mp_recv_line(socket);
            if (strncmp(line, "+OK ", 4) != 0) {
                mp_disconnect(socket);
                unknown("Don't looks like pop: %s", line);
            }
            free(line);

            // Ask for STARTTLS
            send(socket, "STLS\n", 5, 0);
            if (mp_verbose > 3)
                printf("> STLS\n");

            // Check reply
            line = mp_recv_line(socket);
            if (strncmp(line, "+OK ", 4) != 0) {
                mp_disconnect(socket);
                unknown("STARTTLS Error: %s", line);
            }
            free(line);
        } else if (strcmp(starttls, "imap") == 0) {
            char *line;
            int has_starttls = 0;

            // Wait for banner
            line = mp_recv_line(socket);
            if (strncmp(line, "* OK ", 4) != 0) {
                mp_disconnect(socket);
                unknown("Don't looks like imap: %s", line);
            }

            // Ask for CAPABILITY
            send(socket, "a001 CAPABILITY\n", 16, 0);
            if (mp_verbose > 3)
                printf("> a001 CAPABILITY\n");

            // Read CAPABILITY reply
            do {
                free(line);
                line = mp_recv_line(socket);
                if (strncmp(line, "a001 BAD", 8) == 0) {
                    mp_disconnect(socket);
                    critical("CAPABILITY failed: %s", line);
                }
                if (strstr(line, "STARTTLS"))
                    has_starttls = 1;
            } while (line && (strncmp(line, "a001 OK ", 8) != 0));
            free(line);

            if (has_starttls == 0)
                critical("IMAP Server do not offer STARTTLS");

            // Ask for STARTTLS
            send(socket, "a002 STARTTLS\n", 14, 0);
            if (mp_verbose > 3)
                printf("> a002 STARTTLS\n");

            // Check reply
            line = mp_recv_line(socket);
            if (strncmp(line, "a002 OK ", 8) != 0) {
                mp_disconnect(socket);
                unknown("STARTTLS Error: %s", line);
            }
            free(line);
        } else {
            unknown("STARTTLS protocoll %s not known.", starttls);
        }
    }

    // GNUTLS init
    gnutls_global_init();
    gnutls_certificate_allocate_credentials(&xcred);
    gnutls_init(&session, GNUTLS_CLIENT);
    if (servername && sni) {
        gnutls_server_name_set(session, 1, (void *)servername,
                strlen(servername));
        printf("SNI:1\n");
    }
    else if (!is_hostaddr(hostname) && sni){
        gnutls_server_name_set(session, 1, (void *) hostname, strlen(hostname));
        printf("SNI:2\n");
    }
    gnutls_session_set_ptr(session, (void *) hostname);
    gnutls_priority_set_direct(session, "PERFORMANCE", &err);
    gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);

    gnutls_transport_set_ptr(session, (gnutls_transport_ptr_t)(intptr_t)socket);

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

    out = mp_malloc(len + 8);
    mp_sprintf(out, "Cert: %s", buf);

    ret = gnutls_certificate_verify_peers2(session, &cstatus);
    if (ret < 0) {
        mp_disconnect(socket);
        gnutls_deinit (session);
        gnutls_certificate_free_credentials (xcred);
        gnutls_global_deinit ();
        critical("Can't verify cert.");
    }

    if (cstatus & GNUTLS_CERT_EXPIRED) {
        mp_strcat_space(&out, "is expired");
        status = STATE_CRITICAL;
    } else if (cstatus & GNUTLS_CERT_NOT_ACTIVATED) {
        mp_strcat_space(&out, "is not yet activated");
        status = STATE_CRITICAL;
    } else {
        status = get_status((expire-time(0)), expire_thresholds);
        strftime(buf, 200, "expires %F", localtime(&expire));
        mp_strcat_space(&out, buf);
    }
    free(buf);

    for(i = 0; i < ca_files; i++) {
        buf = strsep(&ca_file[i], ":");
        if (ca_file[i] == NULL) {
            ca_file[i] = buf;
        }
        if (mp_verbose >= 1)
            printf("Check file: %s named %s\n", buf, ca_file[i]);
        gnutls_certificate_free_cas(xcred);
        gnutls_certificate_set_x509_trust_file(xcred, ca_file[i], GNUTLS_X509_FMT_PEM);

        ret = gnutls_certificate_verify_peers2(session, &cstatus);
        if ( ret < 0)
            critical("gnutls_certificate_verify_peers2 failed!");
        if ((cstatus & GNUTLS_CERT_INVALID) || (cstatus & GNUTLS_CERT_SIGNER_NOT_FOUND)) {
            mp_strcat_comma(&untrusted, buf);
        }
    }

    if (untrusted != NULL) {
        mp_strcat_space(&out, "untrusted in:");
        mp_strcat_space(&out, untrusted);
        status = STATE_CRITICAL;
    }

    // Dissconnect
    mp_disconnect(socket);
    gnutls_deinit (session);
    gnutls_certificate_free_credentials (xcred);
    gnutls_global_deinit ();

    switch (status) {
        case STATE_OK:
            ok(out);
            break;
        case STATE_WARNING:
            warning(out);
            break;
        case STATE_CRITICAL:
            critical(out);
            break;
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
        {"starttls", required_argument, NULL, MP_LONGOPT_STARTTLS},
        {"sni", required_argument, NULL, (int)'s'},
        {"no-sni", no_argument, NULL, MP_LONGOPT_NOSNI},
        MP_LONGOPTS_WC,
        MP_LONGOPTS_END
    };

    setWarnTime(&expire_thresholds, "30d:");
    setCritTime(&expire_thresholds, "10d:");

    while (1) {
        c = mp_getopt(&argc, &argv, MP_OPTSTR_DEFAULT"H:P:46s:w:c:C:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        getopt_46(c, &ipv);
        getopt_wc_time_at(c, optarg, &expire_thresholds);

        switch (c) {
            /* Plugin opts */
            case MP_LONGOPT_STARTTLS:
                starttls = optarg;
                break;
            case 's':
                sni = 1;
                getopt_host(optarg, &servername);
                break;
            case MP_LONGOPT_NOSNI:
                sni = 0;
                break;
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            /* Port opt */
            case 'P':
                getopt_port(optarg, &port);
                break;
            /* CAs opt */
            case 'C':
                mp_array_push(&ca_file, optarg, &ca_files);
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

    printf("Check description: %s", progdesc);

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    print_help_port("none");
#ifdef USE_IPV6
    print_help_46();
#endif //USE_IPV6
    printf("     --starttls=[PROTO]\n");
    printf("      Use named STARTTLS protocol. (smtp, pop or imap)\n");
    printf(" -s. --sni=SERVERNAME\n");
    printf("      Use SERVERNAME to request Cert by SNI.\n");
    printf("     --no-sni\n");
    printf("      Do not send servername.\n");
    printf(" -C, --trusted-ca=[NAME:]FILE\n");
    printf("      File to read trust-CAs from.\n");
    print_help_warn_time("30 days");
    print_help_crit_time("10 days");
}

/* vim: set ts=4 sw=4 et syn=c : */
