//
// Created by wonicon on 15-5-24.
//

#ifndef MYARP_IPTABLE_H
#define MYARP_IPTABLE_H
#include <stdint.h>
typedef struct _ip_table_entry_
{
    uint32_t dst;
    uint32_t src;
    uint32_t mask;
    char dev[16];
}IPTE, *PIPTE;
void read_table();
void print_ip(uint32_t addr);
IPTE *next_hop(uint32_t dst);

#endif //MYARP_IPTABLE_H
