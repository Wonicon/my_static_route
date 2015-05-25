//
// Created by Wang Huizhe on 15-5-24.
//

#include "arptable.h"
#include "iptable.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct _arp_table_entry_
{
    uint32_t ip;
    uint8_t mac[6];
} ARPENT, *PARPENT;

#define BUFFER_SIZE 128

static int n;
static ARPENT arp_table[BUFFER_SIZE];

void read_arp_cache()
{
    FILE *fd = fopen("arp_cache", "r");
    char buffer[1024];
    uint8_t ip[4];
    char *p;

	fgets(buffer, 1024, fd);
	n = (int)strtol(buffer, NULL, 10);
    printf("arp entries %d\n", n);

    for (int i = 0; i < n; i++) {
        fgets(buffer, 1024, fd);
        p = buffer;
        for (int j = 0; j < 4; j++) {
            ip[j] = (uint8_t)strtoll(p, &p, 10);
            p++;
        }

        arp_table[i].ip = *(uint32_t *)ip;
        for (int j = 0; j < 6; j++) {
            arp_table[i].mac[j] = (uint8_t)strtol(p, &p, 16);
            p++;
        }
    }
}

uint8_t *get_mac(uint32_t ip)
{
	printf("Scan arp cache to find the mac address of %s\n", iptoa(ip));
    for (int i = 0; i < n; i++) {
        if (arp_table[i].ip == ip) {
            printf("Found %s.\n", mactoa(arp_table[i].mac));
            return arp_table[i].mac;
        }
    }

    printf("Failed.\n");
    return NULL;
}

#include <memory.h>
void add_mac(uint32_t ip, uint8_t *mac)
{
    printf("add arp entry: %s %s\n", iptoa(ip), mactoa(mac));
    if (n >= BUFFER_SIZE) {
        return;
    }

    for (int i = 0; i < n; i++) {
        if (arp_table[i].ip == ip) {
            return;
        }
    }
    arp_table[n].ip = ip;
    memcpy(arp_table[n].mac, mac, 6);
    n++;
}
