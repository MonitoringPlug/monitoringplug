/***
 * Monitoring Plugin - check_dhcp.c
 **
 *
 * check_dhcp - Check a DHCP server.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id$
 */

const char *progname  = "check_dhcp";
const char *progdesc  = "Check dhcp server";
const char *progvers  = "0.1";
const char *progcopy  = "2012";
const char *progauth  = "Marius Rieder <marius.rieder@durchmesser.ch>";
const char *progusage = "[-i <DEV>] [-H <HOSTNAME> [--unicast]] [--mac <MAC>]";

/* MP Includes */
#include "mp_common.h"
#include "dhcp_utils.h"
/* Default Includes */
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
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
struct dhcp_pkt *dhcp_packet(int op);
void dhcp_pkt_opt(struct dhcp_pkt *pkt, uint8_t code, uint8_t len, char *data);
char *dhcp_pkt_getopt(struct dhcp_pkt *pkt, uint8_t code, uint8_t len);
unsigned short int ip_checksum(unsigned short int *addr, int len);
ssize_t dhcp_send(int sockfd, struct dhcp_pkt *pkt);
struct dhcp_pkt *dhcp_recv(int sockfd);
char *ip_ntoa(uint8_t *addr);

char *ip_ntoa(uint8_t *addr) {
    struct in_addr *x;
    x = (struct in_addr *)(void *)addr;
    return inet_ntoa(*x);
}

int main (int argc, char **argv) {
    /* Local Vars */
    struct ifreq    ifr;
    int             sock;
    uint8_t         opt;
    struct dhcp_pkt *pkt;
    struct hostent  *hostent = NULL;
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

    pkt = dhcp_packet(BOOTREQUEST);
    opt = DHCPDISCOVER;
    if (request) {
        opt = DHCPREQUEST;
        dhcp_pkt_opt(pkt, DHCPOPT_RequestIP, 4, (char *)&request);
    }
    dhcp_pkt_opt(pkt, DHCPOPT_MessageType, 1, (char *)&opt);
    dhcp_pkt_opt(pkt, DHCPOPT_Class, strlen(progname), (char *)progname);

    if (unicast) {
        ioctl(sock, SIOCGIFADDR, &ifr);
        memcpy(pkt->giaddr, &((struct sockaddr_in *)(void *)&ifr.ifr_addr)->sin_addr, 4);
    }

    int s = dhcp_send(sock, pkt);
    if (s < 0)
        warning("DHCP %d", s);

    if (hostname)
        hostent = gethostbyname(hostname);

    int ansers = 0;
    while ((pkt = dhcp_recv(sock))) {
        ansers++;

        if (hostname) {
            struct in_addr *sid;

            sid = (struct in_addr *)(void *)dhcp_pkt_getopt(pkt, DHCPOPT_ServerId, 4);
            if (sid) {
                if (memcmp(hostent->h_addr, sid, 4) != 0)
                    continue;
            } else {
                if (strcmp(hostname, pkt->sname) != 0)
                    continue;
            }

            uint8_t *t;
            t = (uint8_t *)dhcp_pkt_getopt(pkt, DHCPOPT_MessageType, 1);
            if (type && (*t == type)) {
                switch (type) {
                    case DHCPOFFER:
                        ok("Got offer for %hhd.%hhd.%hhd.%hhd from %s",
                                pkt->yiaddr[0], pkt->yiaddr[1], pkt->yiaddr[2],
                                pkt->yiaddr[3], pkt->sname);
                    case DHCPACK:
                        ok("Got ACK from %s", pkt->sname);
                    case DHCPNAK:
                        ok("Got NAK from %s", pkt->sname);
                };
            } else {
                switch (*t) {
                    case DHCPOFFER:
                        ok("Got offer for %s from %s", ip_ntoa(pkt->yiaddr), pkt->sname);
                    case DHCPACK:
                        ok("Got ACK from %s", pkt->sname);
                    case DHCPNAK:
                        ok("Got NAK from %s", pkt->sname);
                };
            }
        } else {
            uint8_t *t;
            t = (uint8_t *)dhcp_pkt_getopt(pkt, DHCPOPT_MessageType, 1);
            if (type && (*t == type)) {
                switch (type) {
                    case DHCPOFFER:
                        ok("Got offer for %s from %s", ip_ntoa(pkt->yiaddr), pkt->sname);
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
        hostent = gethostbyname(hostname);
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

struct dhcp_pkt *dhcp_packet(int op) {
    struct dhcp_pkt *pkt;

    pkt = mp_malloc(sizeof(struct dhcp_pkt));
    bzero(pkt, sizeof(struct dhcp_pkt));

    pkt->op = op;
    pkt->htype = 1;
    pkt->hlen = maclen;
    pkt->hops = 1;
    pkt->xid = (uint32_t)random();
    pkt->secs = 0;
    pkt->flags = 0x80;
    memcpy(pkt->chaddr, mac, 16);

    return pkt;
}

void dhcp_pkt_opt(struct dhcp_pkt *pkt, uint8_t code, uint8_t len, char *data) {

    if(pkt->optlen == 0) {
        pkt->opts = mp_malloc(len + 4);
        /* Cookie */
        memcpy(pkt->opts, magickcookie, 4);
        pkt->optlen = 4;
    } else {
        pkt->opts = mp_realloc(pkt->opts, pkt->optlen + len + 2);
    }

    pkt->opts[pkt->optlen] = code;
    pkt->optlen++;

    if (len) {
        pkt->opts[pkt->optlen] = len;
        memcpy(&pkt->opts[pkt->optlen+1], data, len);
        pkt->optlen += 1 + len;
    }
}

char *dhcp_pkt_getopt(struct dhcp_pkt *pkt, uint8_t code, uint8_t len) {
    int i = 0;

    if(pkt->optlen == 0)
        return NULL;

    while (i < pkt->optlen) {
    //    printf(" [%d] %d\n", i, pkt->opts[i]);
        if (pkt->opts[i] == code && pkt->opts[i+1] == len)
            return (char *)&(pkt->opts[i+2]);
        i += pkt->opts[i+1] + 2;
    }
    return NULL;
}

// Checksum function
unsigned short int ip_checksum(unsigned short int *addr, int len) {
  int nleft = len;
  int sum = 0;
  unsigned short int *w = addr;
  unsigned short int answer = 0;

  while (nleft > 1) {
    sum += *w++;
    nleft -= sizeof (unsigned short int);
  }

  if (nleft == 1) {
    *(unsigned char *) (&answer) = *(unsigned char *) w;
    sum += answer;
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}
   

ssize_t dhcp_send(int sockfd, struct dhcp_pkt *pkt) {
    struct iovec        iov[4];
    struct msghdr       mh;
    struct sockaddr_ll  device;
    char                *buf;
    struct ip           *ipHdr;
    struct udphdr       *udpHdr;

    // Build msghdr
    bzero(&mh, sizeof(mh));
    mh.msg_name = (caddr_t)&dhcpd;
    mh.msg_namelen = sizeof(dhcpd);
    mh.msg_iov = iov;
    mh.msg_iovlen = 0;

    // Raw sock for broadcast
    if (!unicast) {
        sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if(sockfd < 0) {
            perror("socket() error");
            return -1;
        }

        device.sll_family = AF_PACKET;
        device.sll_protocol = htons (ETH_P_IP);
        memset(device.sll_addr, 0xFF, 6);
        device.sll_halen = htons (6);
        device.sll_ifindex = if_nametoindex (interface);

        mh.msg_name = (caddr_t)&device;
        mh.msg_namelen = sizeof(device);

        // Build IP/UDP Header
        buf = mp_malloc(14 + sizeof(struct ip) + sizeof(struct udphdr));
        bzero(buf, sizeof(buf));
        ipHdr = (struct ip *)(void *)(buf + 14);
        udpHdr = (struct udphdr *)(void *)(buf + 14 + sizeof(struct ip));

        //MAC
        memset(buf, 0xFF, 6);
        memcpy(buf+6, mac, 6);
        buf[12] = ETH_P_IP / 256;
        buf[13] = ETH_P_IP % 256;

        // IP
        ipHdr->ip_v = 4;
        ipHdr->ip_hl = 5;
        ipHdr->ip_tos = 0;
        ipHdr->ip_len =  20 + 8 + 236;
        if (pkt->optlen > 0)
            ipHdr->ip_len += pkt->optlen + 1;
        ipHdr->ip_len =  htons(ipHdr->ip_len);
        ipHdr->ip_off = 0;
        ipHdr->ip_ttl = 20;
        ipHdr->ip_p = IPPROTO_UDP;
        ipHdr->ip_src = dhcpc.sin_addr;
        ipHdr->ip_dst = dhcpd.sin_addr;
        ipHdr->ip_sum = 0;
        ipHdr->ip_sum = ip_checksum ((unsigned short int *) ipHdr, 20);

        // UDP
        udpHdr->source = dhcpc.sin_port;
        udpHdr->dest = dhcpd.sin_port;
        udpHdr->len = htons(ntohs(ipHdr->ip_len)-20);

        iov[mh.msg_iovlen].iov_base = (caddr_t)buf;
        iov[mh.msg_iovlen].iov_len = 42;
        mh.msg_iovlen += 1;
    }

    iov[mh.msg_iovlen].iov_base = (caddr_t)pkt;
    iov[mh.msg_iovlen].iov_len = 236;
    mh.msg_iovlen++;

    if (pkt->optlen > 0) {
        dhcp_pkt_opt(pkt, 255, 0, NULL);
        iov[mh.msg_iovlen].iov_base = (caddr_t)pkt->opts;
        iov[mh.msg_iovlen].iov_len = pkt->optlen;
        mh.msg_iovlen++;
    }

    int rc = sendmsg(sockfd, &mh, 0);

    xid = pkt->xid;

    if (rc == -1) {
        perror("sendmsg failed");
    }

    printf("Sendt %d\n", rc);

    return rc;
}

struct dhcp_pkt *dhcp_recv(int sockfd) {
    struct dhcp_pkt *pkt;
    struct timeval  timeout;
    struct msghdr   mh;
    struct iovec    iov[2];
    uint8_t         cookie[4];
    int             len;
    fd_set read;

    FD_ZERO(&read);
    FD_SET(sockfd,&read);

    timeout.tv_sec=5;
    timeout.tv_usec=0;

    while (select(sockfd+1, &read, NULL, NULL, &timeout) > 0) {
        if (mp_verbose > 2)
            printf("Read packet\n");
        if (!FD_ISSET(sockfd, &read))
            continue;

        pkt = mp_malloc(sizeof(struct dhcp_pkt));
        bzero(pkt, sizeof(struct dhcp_pkt));
        pkt->opts = mp_malloc(512);
        bzero(pkt->opts, DHCPMINOPTLEN);

        iov[0].iov_base = (caddr_t)pkt;
        iov[0].iov_len = 236;
        iov[1].iov_base = (caddr_t)cookie;
        iov[1].iov_len = 4;
        iov[2].iov_base = (caddr_t)pkt->opts;
        iov[2].iov_len = DHCPMINOPTLEN;

        bzero(&mh, sizeof(mh)); 
        mh.msg_iov = iov;
        mh.msg_iovlen = 3;

        len = recvmsg(sockfd, &mh, 0);
        if (len < 0) {
            perror("recvmsg");
            free(pkt->opts);
            free(pkt);
            return NULL;
        }

        pkt->optlen = len-240;

        // Check op
        if (pkt->op != BOOTREPLY) {
            if (mp_verbose > 2)
                printf("No BOOTREPLY message.");
            free(pkt->opts);
            free(pkt);
            continue;
        }

        // Check xid
        if (pkt->xid != xid) {
            if (mp_verbose > 2)
                printf("No BOOTREPLY message.");
            free(pkt->opts);
            free(pkt);
            continue;
        }

        // Check cookie
        if (pkt->optlen > 0 &&  memcmp(cookie, magickcookie, 4) != 0) {
            if (mp_verbose > 2)
                printf("Illegal cookie Dropping package.");
            free(pkt->opts);
            free(pkt);
            continue;
        }

        // Check opts
        if (pkt->optlen > 0) {
            int i = 0;
            while (i < pkt->optlen) {
                if (pkt->opts[i] == DHCPOPT_Pad) {
                    i++;
                    continue;
                }
                if (pkt->opts[i] == DHCPOPT_End) {
                    break;
                }
                i += pkt->opts[i+1] + 2;
            }
            if (i >= pkt->optlen) {
                if (mp_verbose > 2)
                    printf("DHCP Options without end");
                free(pkt->opts);
                free(pkt);
                continue;
            }
        }

        if (mp_verbose > 1) {
            uint8_t *i;
            i = (uint8_t *)dhcp_pkt_getopt(pkt, DHCPOPT_MessageType, 1);
            if (i != NULL) {
                switch(*i) {
                    case DHCPDISCOVER:
                        printf("DHCP Discover\n");
                        break;
                    case DHCPOFFER:
                        printf("DHCP Offer\n");
                        break;
                    case DHCPREQUEST:
                        printf("DHCP Request\n");
                        break;
                    case DHCPDECLINE:
                        printf("DHCP Declinc\n");
                        break;
                    case DHCPACK:
                        printf("DHCP Ack\n");
                        break;
                    case DHCPNAK:
                        printf("DHCP Nak\n");
                        break;
                    case DHCPRELEASE:
                        printf("DHCP Release\n");
                        break;
                    case DHCPINFORM:
                        printf("DHCP Inform\n");
                        break;
                }
            } else {
                printf("BOOTP Reply\n");
            }

            printf(" Client Addr: %s\n", ip_ntoa(pkt->ciaddr));
            printf(" Your Addr:   %s\n", ip_ntoa(pkt->yiaddr));
            printf(" Next Server: %s\n", ip_ntoa(pkt->siaddr));
            printf(" Gateway:     %s\n", ip_ntoa(pkt->giaddr));
            printf(" Client MAC:  %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n",
                    pkt->chaddr[0], pkt->chaddr[1], pkt->chaddr[2],
                    pkt->chaddr[3], pkt->chaddr[4], pkt->chaddr[5]);
            printf(" Servername:  %s\n", pkt->sname);
            printf(" Bootfile:    %s\n", pkt->file);
        }

        return pkt;
    }

    return NULL;
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
        c = getopt_long (argc, argv, MP_OPTSTR_DEFAULT"H:i:m:ubr:T:t:", longopts, &option);

        if (c == -1 || c == EOF)
            break;

        switch(c) {
            /* Default opts */
            MP_GETOPTS_DEFAULT
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
            /* Timeout opt */
            case 't':
                getopt_timeout(optarg);
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
    printf("  Run without hostname to ask broadcast and accept only answers from given host.\n\n");
    printf(" Mode 3:\n");
    printf("  Run without hostname and unicast flag. Act as forwarder/redirector and ask\n");
    printf("  the server directly in the name of a client.");

    printf("\n\n");

    print_usage();

    print_help_default();
    print_help_host();
    printf(" -i, --interface\n");
    printf("      Network interface to use. (Default: eth0)\n");
    printf(" -m, --mac=MAC\n");
    printf("      MAC Address to use. (Default: from interface)\n");
    printf(" -u, --unicast\n");
    printf("      Run in unicast mode.\n");
    printf(" -b, --broadcast\n");
    printf("      Run in broadcast mode.\n");


}

/* vim: set ts=4 sw=4 et syn=c : */
