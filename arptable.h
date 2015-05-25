//
// Created by wonicon on 15-5-24.
//

#ifndef MYARP_ARPTABLE_H
#define MYARP_ARPTABLE_H
#include <stdint.h>
void read_arp_cache();
uint8_t *get_mac(uint32_t ip);
void add_mac(uint32_t ip, uint8_t *mac);
#endif //MYARP_ARPTABLE_H
