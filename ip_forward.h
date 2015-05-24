//
// Created by wonicon on 15-5-24.
//

#ifndef MYARP_IP_FORWARD_H
#define MYARP_IP_FORWARD_H
#include <stdint.h>
void init_device_table();
void ip_route(int sock, uint8_t *packet, int size);
void forward(int sock, const char *device, uint8_t *mac, uint8_t *packet, int n);
#endif //MYARP_IP_FORWARD_H
