#ifndef _ARPXXX_H_
#define _ARPXXX_H_

#include <stdint.h>

struct arp_packet {
    uint8_t dmac[6];  //  0x00 destination mac address
    uint8_t smac[6];  //  0x06 source mac address
    uint16_t type;    //  0x0c packet type

    uint16_t htype;   //  0x0e hardware type
    uint16_t ptype;   //  0x10 protocol type
    uint8_t hlen;     //  0x12 hardware address length
    uint8_t plen;     //  0x13 protocol address length
    uint16_t opt;     //  0x14 ARP opcode
    uint8_t sha[6];   //  0x16 sender's hardware address
    uint8_t spa[4];   //  0x1c sender's protocol address
    uint8_t tha[6];   //  0x20 target's hardware address
    uint8_t tpa[4];   //  0x26 target's protocol address
    uint8_t pad[18];  //  0x2a pad for min ethernet payload
                      //  0x3c = 60
} PACKED;

int make_arp(
        uint32_t    target_ip,
        uint32_t    sender_ip,
        uint8_t    *sender_mac,
        const char *interface);

int read_interface(
        const char *interface,
        int *ifindex,
        uint32_t *addr,
        uint8_t *mac);

#endif
