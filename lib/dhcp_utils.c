/***
 * Monitoring Plugin - dhcp_utils.c
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
#include "mp_net.h"
#include "dhcp_utils.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_packet.h>

const uint8_t magickcookie[4] = {0x63, 0x82, 0x53, 0x63};
const char *mp_dhcp_message_types[] = {
    "",
    "Discover",  /**< 1 - DHCPDISCOVER */
    "Offer",     /**< 2 - DHCPOFFER */
    "Request",   /**< 3 - DHCPREQUEST */
    "Decline",   /**< 4 - DHCPDECLINE */
    "Ack",       /**< 5 - DHCPACK */
    "Nak",       /**< 6 - DHCPNAK */
    "Release",   /**< 7 - DHCPRELEASE */
    "Inform",    /**< 8 - DHCPINFORM */
};

ssize_t mp_dhcp_send(int sockfd, struct dhcp_pkt *pkt, struct sockaddr_in *from,
        struct sockaddr_in *to, const char *interface, int unicast) {
    struct iovec        iov[4];
    struct msghdr       mh;
    struct sockaddr_ll  device;
    ssize_t             len;
    char                *buf;
    struct ip           *ipHdr;
    struct udphdr       *udpHdr;

    // Build msghdr
    bzero(&mh, sizeof(mh));
    mh.msg_name = (caddr_t)to;
    mh.msg_namelen = sizeof(*to);
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
        device.sll_ifindex = if_nametoindex(interface);

        mh.msg_name = (caddr_t)&device;
        mh.msg_namelen = sizeof(device);

        // Build IP/UDP Header
        buf = mp_malloc(14 + sizeof(struct ip) + sizeof(struct udphdr));
        bzero(buf, sizeof(buf));
        ipHdr = (struct ip *)(void *)(buf + 14);
        udpHdr = (struct udphdr *)(void *)(buf + 14 + sizeof(struct ip));

        //MAC
        memset(buf, 0xFF, 6);
        memcpy(buf+6, pkt->chaddr, 6);
        buf[12] = ETH_P_IP / 256;
        buf[13] = ETH_P_IP % 256;

        // IP
        ipHdr->ip_v = 4;
        ipHdr->ip_hl = 5;
        ipHdr->ip_tos = 0;
        ipHdr->ip_len =  20 + 8 + 236;
        if (pkt->optlen > 0)
            ipHdr->ip_len += pkt->optlen + 4;
        ipHdr->ip_len =  htons(ipHdr->ip_len);
        ipHdr->ip_off = 0;
        ipHdr->ip_ttl = 20;
        ipHdr->ip_p = IPPROTO_UDP;
        ipHdr->ip_src = from->sin_addr;
        ipHdr->ip_dst = to->sin_addr;
        ipHdr->ip_sum = 0;
        ipHdr->ip_sum = mp_ip_csum((unsigned short int *)ipHdr, 20);

        // UDP
        udpHdr->source = from->sin_port;
        udpHdr->dest = to->sin_port;
        udpHdr->len = htons(ntohs(ipHdr->ip_len)-20);

        iov[mh.msg_iovlen].iov_base = (caddr_t)buf;
        iov[mh.msg_iovlen].iov_len = 42;
        mh.msg_iovlen += 1;
    }


    iov[mh.msg_iovlen].iov_base = (caddr_t)pkt;
    iov[mh.msg_iovlen].iov_len = 236;
    mh.msg_iovlen++;

    if (pkt->optlen > 0) {
        iov[mh.msg_iovlen].iov_base = (caddr_t)magickcookie;
        iov[mh.msg_iovlen].iov_len = 4;
        mh.msg_iovlen++;
        iov[mh.msg_iovlen].iov_base = (caddr_t)pkt->opts;
        iov[mh.msg_iovlen].iov_len = pkt->optlen;
        mh.msg_iovlen++;
    }

    len = sendmsg(sockfd, &mh, 0);

    if (len == -1) {
        perror("sendmsg failed");
    }

    if (mp_verbose > 2) {
        printf("Sendt %db\n", (int)len);
        mp_dhcp_pkt_dump(pkt);
    }

    return len;
}

struct dhcp_pkt *mp_dhcp_recv(int sockfd, uint32_t xid) {
    struct dhcp_pkt *pkt;
    struct timeval  timeout;
    struct msghdr   mh;
    struct iovec    iov[3];
    uint8_t         cookie[4];
    int             len;
    fd_set read;

    FD_ZERO(&read);
    FD_SET(sockfd,&read);

    timeout.tv_sec = mp_timeout;
    timeout.tv_usec = 0;

    while (select(sockfd+1, &read, NULL, NULL, &timeout) > 0) {
        if (!FD_ISSET(sockfd, &read))
            continue;

        pkt = mp_malloc(sizeof(struct dhcp_pkt));
        bzero(pkt, sizeof(struct dhcp_pkt));
        pkt->opts = mp_malloc(DHCPMINOPTLEN);
        bzero(pkt->opts, DHCPMINOPTLEN);

        iov[0].iov_base = (caddr_t)pkt;
        iov[0].iov_len  = 236;
        iov[1].iov_base = (caddr_t)cookie;
        iov[1].iov_len  = 4;
        iov[2].iov_base = (caddr_t)pkt->opts;
        iov[2].iov_len  = DHCPMINOPTLEN;

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

        if (mp_verbose > 2)
            printf("Read packet: %db\n", len);

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
                printf("ID missmatch.");
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
            mp_dhcp_pkt_dump(pkt);
        }

        return pkt;
    }

    return NULL;
}

void mp_dhcp_pkt_opt(struct dhcp_pkt *pkt, uint8_t code, uint8_t len, char *data) {
    if(pkt->optlen == 0) {
        pkt->opts = mp_malloc(len + 3);
        pkt->optlen = 1;
    } else {
        pkt->opts = mp_realloc(pkt->opts, pkt->optlen + len + 2);
    }

    pkt->opts[(pkt->optlen)-1] = code;

    if (len) {
        pkt->opts[pkt->optlen] = len;
        memcpy(&pkt->opts[pkt->optlen+1], data, len);
        pkt->optlen += 2 + len;
    }

    pkt->opts[(pkt->optlen)-1] = DHCPOPT_End;
}

struct dhcp_pkt_opt *mp_dhcp_pkt_getopt(struct dhcp_pkt *pkt, uint8_t code) {
    struct dhcp_pkt_opt *opt;
    int i = 0;

    if(pkt->optlen == 0)
        return NULL;

    while (i < pkt->optlen) {
        opt = (void *)(pkt->opts) + i;

        if (opt->code == code)
            return opt;

        switch (opt->code) {
            case DHCPOPT_Pad:
                i++;
                continue;
            case DHCPOPT_End:
                return NULL;
        };

        i += 2 + opt->len;
    }
    return NULL;
}

void mp_dhcp_pkt_dump(struct dhcp_pkt *pkt) {
    uint8_t i;
    struct dhcp_pkt_opt *opt;
    opt = mp_dhcp_pkt_getopt(pkt, DHCPOPT_MessageType);

    if (opt != NULL) {
        printf("[DHCP %s]\n", mp_dhcp_message_types[opt->data.uint8]);
    } else if (pkt->op == BOOTREQUEST) {
        printf("[BOOTP Request]\n");
    } else {
        printf("[BOOTP Reply]\n");
    }

    printf(" Client Addr: %s\n", inet_ntoa(pkt->ciaddr));
    printf(" Your Addr:   %s\n", inet_ntoa(pkt->yiaddr));
    printf(" Next Server: %s\n", inet_ntoa(pkt->siaddr));
    printf(" Gateway:     %s\n", inet_ntoa(pkt->giaddr));
    printf(" Client MAC:  %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n",
            pkt->chaddr[0], pkt->chaddr[1], pkt->chaddr[2],
            pkt->chaddr[3], pkt->chaddr[4], pkt->chaddr[5]);
    printf(" Servername:  %s\n", pkt->sname);
    printf(" Bootfile:    %s\n", pkt->file);

    if (pkt->optlen > 0) {
        printf(" Options:\n");

        i = 0;
        while(i < pkt->optlen) {
            opt = (void *)(pkt->opts) + i;

            switch (opt->code) {
                case DHCPOPT_Pad:
                    printf("  [PAD]\n");
                    i++;
                    continue;
                case DHCPOPT_Subnetmask:
                    printf("  [Subnetmask] %s\n", inet_ntoa(opt->data.inaddr));
                    break;
                case DHCPOPT_Router:
                    printf("  [Router] %s\n", inet_ntoa(opt->data.inaddr));
                    break;
                case DHCPOPT_DNS:
                    printf("  [Name Server] %s\n", inet_ntoa(opt->data.inaddr));
                    break;
                case DHCPOPT_Broadcast:
                    printf("  [Broadcast] %s\n", inet_ntoa(opt->data.inaddr));
                    break;
                case DHCPOPT_Hostname:
                    printf("  [Host Name] %.*s\n", opt->len, &(opt->data.string));
                    break;
                case DHCPOPT_MessageType:
                    printf("  [DHCP Type] %s\n", mp_dhcp_message_types[opt->data.uint8]);
                    break;
                case DHCPOPT_ServerId:
                    printf("  [DHCPD ID] %s\n", inet_ntoa(opt->data.inaddr));
                    break;
                case DHCPOPT_Class:
                    printf("  [Client Class] %.*s\n", opt->len, &(opt->data.string));
                    break;
                case DHCPOPT_End:
                    printf("  [END]\n");
                    i = pkt->optlen + 1;
                    continue;
                default:
                    printf("  [%3d:%3d]\n", opt->code, opt->len);
                    break;
            }

            i += opt->len + 2;
        }
    }
}

/* vim: set ts=4 sw=4 et syn=c : */
