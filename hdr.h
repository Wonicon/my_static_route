//
// Created by Wang Huizhe on 15-5-23.
//

#ifndef MYARP_HDR_H
#include <stdint.h>
#include <linux/if_arp.h>   // arphdr
#define EPT_IP 0x0800 /* type: IP */
#define EPT_ARP 0x0806 /* type: ARP */
#define ARP_REQUEST 0x0001 /* ARP request */
#define ARP_REPLY 0x0002 /* ARP reply */

typedef struct _ETH_HEAD_ {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t type;
} ETH_HEAD, *P_ETH_HEAD;

typedef struct arp_datagram /* Focus on TCP/IP with Ethernet II */
{
    struct arphdr hdr;
    uint8_t sha[6];
    uint8_t spa[4];
    uint8_t tha[6];
    uint8_t tpa[4];
} ARP_DATAGRAM, *P_ARP_DATAGRAM;

typedef struct arp_packet
{
    ETH_HEAD eth;
    ARP_DATAGRAM arp;
} ARP_PACKET, P_ARP_PACKET;
#define MYARP_HDR_H

#endif //MYARP_HDR_H
