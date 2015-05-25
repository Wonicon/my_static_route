#ifndef _ARP_H_
#define _ARP_H_

struct arg_packet {
    uint8_t dmac[6];
    uint8_t smac[6];
    uint16_t type;

    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t opt;
    uint8_t sha[6];
    uint8_t spa[6];
    uint8_t tha[6];
    uint8_t tpa[6];
}
#endif
