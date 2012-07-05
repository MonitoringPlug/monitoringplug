/***
 * Monitoring Plugin - dhcp_utils.h
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

#ifndef DHCP_UTILS_H_
#define DHCP_UTILS_H_

#include <stdint.h>
#include <netinet/in.h>

/* The global dhcp vars. */
/** DHCP magick cookie. */
extern const uint8_t magickcookie[4];
/** DHCP message type lookup table. */
extern const char *mp_dhcp_message_types[];

/** BOOTP Operations */
enum {
   BOOTREQUEST  = 1,
   BOOTREPLY    = 2
};

/** DHCP Options */
enum {
   DHCPOPT_Pad          = 0,    /**<   0 - Padding */
   DHCPOPT_Subnetmask   = 1,    /**<   1 - Subnetmask */
   DHCPOPT_Router       = 3,    /**<   3 - Default Gateway */
   DHCPOPT_DNS          = 6,    /**<   6 - DNS Server(s) */
   DHCPOPT_Hostname     = 12,   /**<  12 - Hostname */
   DHCPOPT_Broadcast    = 28,   /**<  28 - Broadcast Address */
   DHCPOPT_RequestIP    = 50,   /**<  50 - Requested IP */
   DHCPOPT_OptOverload  = 52,   /**<  52 - Option overloading */
   DHCPOPT_MessageType  = 53,   /**<  53 - Message type */
   DHCPOPT_ServerId     = 54,   /**<  54 - Server ID */
   DHCPOPT_MsgSize      = 57,   /**<  57 - Message size */
   DHCPOPT_Class        = 60,   /**<  60 - Class */
   DHCPOPT_End          = 255   /**< 255 - End */
};

/** Default return values for functions  */
enum {
   DHCPDISCOVER = 1,
   DHCPOFFER    = 2,
   DHCPREQUEST  = 3,
   DHCPDECLINE  = 4,
   DHCPACK  = 5,
   DHCPNAK  = 6,
   DHCPRELEASE  = 7,
   DHCPINFORM   = 8
};

/** BOOTP/DHCP packet struct */
struct dhcp_pkt {
    uint8_t     op;
    uint8_t     htype;
    uint8_t     hlen;
    uint8_t     hops;
    uint32_t    xid;
    uint16_t    secs;
    uint16_t    flags;
    struct in_addr  ciaddr;
    struct in_addr  yiaddr;
    struct in_addr  siaddr;
    struct in_addr  giaddr;
    uint8_t     chaddr[16];
    char        sname[64];
    char        file[128];
    uint8_t     *opts;
    int         optlen;
};

struct dhcp_pkt_opt {
    uint8_t     code;
    uint8_t     len;
    union {
        char            string;
        uint8_t         uint8;
        struct in_addr  inaddr;
    } data;
} __attribute__((__packed__));

#define DHCPMINOPTLEN 312

/**
 * Send DHCP packet
 * \para[in] sockfd Network socket file descriptor to send with.
 * \para[in] pkt \ref dhcp_pkt to send.
 * \para[in] from Network packet source address.
 * \para[in] to Network packet destination address.
 * \para[in] interface Name of the network interface to send throu.
 * \para[in] unicast Send as unicast.
 */
ssize_t mp_dhcp_send(int sockfd, struct dhcp_pkt *pkt,
        struct sockaddr_in *from, struct sockaddr_in *to,
        const char *interface, int unicast);

/**
 * Receive DHCP packet
 * \para[in] sockfd Network socket file descriptor to receive from.
 * \pata[in] xid XID to receive packets for.
 * \return Return \ref dhcp_pkt or NULL.
 */
struct dhcp_pkt *mp_dhcp_recv(int sockfd, uint32_t xid);

/**
 * Append opt to DHCP Packet
 * \para[in|out] pkt DHCP packet to add option to.
 * \para[in] code DHCP Option code to add.
 * \para[in] len Length of the data to add.
 * \para[in] data Data to add.
 */
void mp_dhcp_pkt_opt(struct dhcp_pkt *pkt, uint8_t code, uint8_t len, char *data);

/**
 * Lookup opt from DHCP Packet
 * \para[in] pkt DHCP packet to get opt of.
 * \para[in] code DHCP Option code.
 * \return Return the \ref dhcp_pkt_opt or NULL.
 */
struct dhcp_pkt_opt *mp_dhcp_pkt_getopt(struct dhcp_pkt *pkt, uint8_t code);

/**
 * Dump DHCP Packet to stdout
 * \para[in] pkt DHCP packet to dump.
 */
void mp_dhcp_pkt_dump(struct dhcp_pkt *pkt);

#endif /* DHCP_UTILS_H_ */

/* vim: set ts=4 sw=4 et syn=c : */
