//
// Created by Wang Huizhe on 15-5-24.
//

#include "arptable.h"
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

#include "iptable.h"
uint8_t *get_mac(uint32_t ip)
{
	printf("scan arp cache\n");
	printf("dest: ");
	print_ip(ip);
	printf("\n");
    for (int i = 0; i < n; i++) {
        if (arp_table[i].ip == ip) {
            return arp_table[i].mac;
        }
    }

    return NULL;
}
