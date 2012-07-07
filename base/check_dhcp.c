/***
 * Monitoring Plugin - check_dhcp.c
 **
 *
 * check_dhcp - Check dhcp server.
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

const char *progname  = "check_dhcp";
const char *progdesc  = "Check dhcp server.";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-i <DEV>] [-H <HOSTNAME> [--unicast]] [--mac <MAC>]";

/* MP Includes */
#include "mp_common.h"
#include "mp_net.h"
#include "dhcp_utils.h"
/* Default Includes */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <time.h>

/* Global Vars */
const char *hostname = NULL;
struct sockaddr_in dhcpd;
struct sockaddr_in dhcpc;
uint8_t mac[16];
int maclen = 0;
const char *interface = "eth0";
int unicast = 0;
in_addr_t request = 0;
uint8_t type = 0;
uint32_t xid;

/* Function prototype */
int dhcp_setup();
unsigned short int ip_checksum(unsigned short int *addr, int len);

int main (int argc, char **argv) {
    /* Local Vars */
    struct ifreq    ifr;
    int             sock;
    ssize_t             s;
    uint8_t             val;
    struct dhcp_pkt     *pkt;
    struct dhcp_pkt_opt *opt;
    struct hostent      *hostent = NULL;
    //uid_t           uid;

    /* Set signal handling and alarm */
    if (signal(SIGALRM, timeout_alarm_handler) == SIG_ERR)
        critical("Setup SIGALRM trap faild!");

    /* Process check arguments */
    if (process_arguments(argc, argv) != OK)
        unknown("Parsing arguments faild!");

    // Need to be root
    mp_noneroot_die();

    // Check hostname
    if (hostname) {
        hostent = gethostbyname(hostname);
        if (!hostent)
            unknown("Host %s not found", hostname);
    }

    /* Start plugin timeout */
    alarm(mp_timeout);

    sock = dhcp_setup();
    if (sock < 0)
        warning("socket setup faild");

    // Read MAC of interface if not specified
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
    ioctl(sock, SIOCGIFHWADDR, &ifr);

    if (maclen == 0) {
        bzero(mac, 16);
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
        maclen = 6;
    }

    // Create XID
    srandom(time(NULL));
    xid = (uint32_t)random();

    // Create Request
    pkt = mp_malloc(sizeof(struct dhcp_pkt));
    bzero(pkt, sizeof(struct dhcp_pkt));

    pkt->op = BOOTREQUEST;
    pkt->htype = 1;
    pkt->hlen = maclen;
    pkt->hops = 1;
    pkt->xid = xid;
    pkt->secs = 0;
    pkt->flags = unicast ? 0x00: 0x80;
    memcpy(pkt->chaddr, mac, 16);

    val = DHCPDISCOVER;
    if (request) {
        val = DHCPREQUEST;
        mp_dhcp_pkt_opt(pkt, DHCPOPT_RequestIP, 4, (char *)&request);
    }
    mp_dhcp_pkt_opt(pkt, DHCPOPT_MessageType, 1, (char *)&val);
    mp_dhcp_pkt_opt(pkt, DHCPOPT_Class, strlen(progname), (char *)progname);

    if (unicast) {
        ioctl(sock, SIOCGIFADDR, &ifr);
        memcpy(&pkt->giaddr, &((struct sockaddr_in *)(void *)&ifr.ifr_addr)->sin_addr, 4);
    }

    s = mp_dhcp_send(sock, pkt, &dhcpc, &dhcpd, interface, unicast);
    if (s < 0)
        warning("DHCP packet not sent");

    if (hostname)
        hostent = gethostbyname(hostname);

    int ansers = 0;
    while ((pkt = mp_dhcp_recv(sock, xid))) {
        ansers++;

        if (hostname && hostent) {
            opt = mp_dhcp_pkt_getopt(pkt, DHCPOPT_ServerId);
            if (opt && hostent) {
                if (memcmp(hostent->h_addr, &(opt->data.inaddr), 4) != 0)
                    continue;
            } else {
                if (strcmp(hostname, pkt->sname) != 0)
                    continue;
            }

            opt = mp_dhcp_pkt_getopt(pkt, DHCPOPT_MessageType);
            if (!opt) {
                continue;
            } else if (opt->data.uint8 == type) {
                switch (type) {
                    case DHCPOFFER:
                        ok("Got offer for %s from %s", inet_ntoa(pkt->yiaddr), pkt->sname);
                    case DHCPACK:
                        ok("Got ACK from %s", pkt->sname);
                    case DHCPNAK:
                        ok("Got NAK from %s", pkt->sname);
                };
            } else {
                switch (opt->data.uint8) {
                    case DHCPOFFER:
                        ok("Got offer for %s from %s", inet_ntoa(pkt->yiaddr), pkt->sname);
                    case DHCPACK:
                        ok("Got ACK from %s", pkt->sname);
                    case DHCPNAK:
                        ok("Got NAK from %s", pkt->sname);
                };
            }
        } else {
            opt = mp_dhcp_pkt_getopt(pkt, DHCPOPT_MessageType);
            if (!opt) {
                continue;
            } else if (opt->data.uint8 == type) {
                switch (type) {
                    case DHCPOFFER:
                        ok("Got offer for %s from %s", inet_ntoa(pkt->yiaddr), pkt->sname);
                    case DHCPACK:
                        ok("Got ACK from %s", pkt->sname);
                    case DHCPNAK:
                        ok("Got NAK from %s", pkt->sname);
                };
            }
        }

        free(pkt->opts);
        free(pkt);
    }

    if (hostname) {
        critical("Got no DHCP reply from %s", hostname);
    } else {
        switch (type) {
            case DHCPOFFER:
                ok("No DHCP Offer received.");
            case DHCPACK:
                ok("No DHCP Ack received.");
            case DHCPNAK:
                ok("No DHCP Nak received.");
        }
    }

    critical("You should never reach this point.");
}

int dhcp_setup() {
    struct servent *servent;
    struct hostent *hostent;
    struct ifreq ifr;
    int sock = -1;
    int one = 1;

    /* dhcpd  sockaddr */
    dhcpd.sin_family = AF_INET;
    if (unicast) {
        if ((hostent = gethostbyname(hostname)) == NULL) {
            perror("gethostbyname");
            return -1;
        }
        memcpy(&dhcpd.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
    } else {
        dhcpd.sin_addr.s_addr = INADDR_BROADCAST;
    }
    servent = getservbyname("bootps", 0);
    dhcpd.sin_port = servent->s_port;

    /* dhcpc sockaddr */
    dhcpc.sin_family = AF_INET;
    dhcpc.sin_addr.s_addr = INADDR_ANY;
    if (!unicast) {
        servent = getservbyname("bootpc", 0);
    }
    dhcpc.sin_port = servent->s_port;
    bzero(dhcpc.sin_zero, sizeof(dhcpc.sin_zero));

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1)
        critical("Can't open udp/ip socket");

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof one) < 0)
        critical("Set socket option faild");

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&one, sizeof one) < 0)
        critical("Set socket to broadcast mode faild");

    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0)
        critical("Bind socket to '%s' faild", interface);

    if (bind(sock, (struct sockaddr *)&dhcpc, sizeof(dhcpc)) < 0) {
        perror("bind");
        critical("Socket bind faild.");
    }

    return sock;
}

int process_arguments (int argc, char **argv) {
    int c;
    int option = 0;

    static struct option longopts[] = {
        MP_LONGOPTS_DEFAULT,
        MP_LONGOPTS_HOST,
        {"interface", required_argument, NULL, (int)'i'},
        {"mac", required_argument, NULL, (int)'m'},
        {"unicast", no_argument, NULL, (int)'u'},
        {"broadcast", no_argument, NULL, (int)'b'},
        {"request", required_argument, NULL, (int)'r'},
        {"type", required_argument, NULL, (int)'T'},
        MP_LONGOPTS_END
    };

    while (1) {
        c = mp_getopt(argc, argv, MP_OPTSTR_DEFAULT"H:i:m:ubr:T:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch(c) {
            /* Default opts */
            /* Hostname opt */
            case 'H':
                getopt_host(optarg, &hostname);
                break;
            case 'i':
                interface = optarg;
            case 'm': {
                memset(&mac,0,sizeof(mac));
                maclen = sscanf(optarg, "%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8":%"SCNx8,
                        &mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5],
                        &mac[6],&mac[7],&mac[8],&mac[9],&mac[10],&mac[11],
                        &mac[12],&mac[13],&mac[14],&mac[15]);
                break;
                      }
            case 'u':
                unicast = 1;
                break;
            case 'b':
                unicast = 0;
                break;
            case 'r':
                if (is_hostaddr(optarg)) {
                    request = inet_addr(optarg);
                } else {
                    usage("Request option must be a ip address");
                }
                break;
            case 'T':
                if (strcasecmp(optarg, "offer") == 0) {
                    type = DHCPOFFER;
                } else if (strcasecmp(optarg, "ack") == 0) {
                    type = DHCPACK;
                } else if (strcasecmp(optarg, "nak") == 0) {
                    type = DHCPNAK;
                } else {
                    usage("Unknown type '%s'", optarg);
                }
                break;
        }

    }

    if (unicast && !hostname)
        usage("Unicast require hostname");

    if (type == 0) {
        type = DHCPOFFER;
        if (request)
            type = DHCPACK;
    }

    return(OK);
}

void print_help (void) {
    print_revision();
    print_copyright();

    printf("\n");

    printf("Check description: %s", progdesc);

    printf("\n\n");

    printf(" Mode 1:\n");
    printf("  Run without hostname to ask broadcast and accept first answer.\n\n");
    printf(" Mode 2:\n");
    printf("  Run with hostname to ask broadcast and accept only answers from given host.\n\n");
    printf(" Mode 3:\n");
    printf("  Run with hostname and unicast flag. Act as forwarder/redirector and ask\n");
    printf("  the server directly in the name of a client.");

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    printf(" -i, --interface=DEV\n");
    printf("      Network interface to use. (Default: eth0)\n");
    printf(" -m, --mac=MAC\n");
    printf("      MAC Address to use. (Default: from interface)\n");
    printf(" -u, --unicast\n");
    printf("      Run in unicast mode.\n");
    printf(" -b, --broadcast\n");
    printf("      Run in broadcast mode.\n");


}

/* vim: set ts=4 sw=4 et syn=c : */
