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

struct wait_packet {
    uint32_t ip;
    int size;
    uint8_t *packet;
    const char *device;
} wait_buffer[BUFFER_SIZE];

int last_index = 0;
#include <malloc.h>
void init_wait_buffer()
{
    memset(wait_buffer, 0, sizeof(wait_buffer));
}

void packet_wait(uint32_t ip, uint8_t *pac, int size, const char *device)
{
    int i;
    for (i = 0; i < BUFFER_SIZE; i++) {
        if (wait_buffer[i].ip == 0) {
            break;
        }
    }
    if (i < BUFFER_SIZE) {
        wait_buffer[i].ip = ip;
        wait_buffer[i].size = size;
        wait_buffer[i].packet = (void *)malloc(size);
        memcpy(wait_buffer[i].packet, pac, size);
        wait_buffer[i].device = device;
    }
    if (i > last_index)
        last_index = i;
}

extern int sock_fd;
void forward(int sock, const char *device, uint8_t *mac, uint8_t *packet, int n);
void resend_packet(uint32_t ip, uint8_t *mac)
{
    for (int i = 0; i <= last_index; i++) {
        if (wait_buffer[i].ip == ip) {
            forward(sock_fd,
                    wait_buffer[i].device,
                    mac,
                    wait_buffer[i].packet,
                    wait_buffer[i].size);
            free(wait_buffer[i].packet);
            wait_buffer[i].ip = 0;
        }
    }
}
