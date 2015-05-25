//
// Created by Wang Huizhe on 15-5-23.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>     // socket()
#include <linux/if_ether.h> // ETH_P_ALL
#include <arpa/inet.h>      // htons


#include "hdr.h"
#include "iptable.h"
#include "arptable.h"
#include "ip_forward.h"
#include "debug.h"          // mactoa()
#include "simple_arp.h"
#define BUFFER_MAX 2048

int sock_fd;
void resend_packet(uint32_t ip, uint8_t *mac);
void main_loop()
{
    ssize_t n_read;
    uint8_t buffer[BUFFER_MAX];
    while (1) {
        n_read = recvfrom(sock_fd, buffer, BUFFER_MAX, 0, NULL, NULL);
        if (n_read < 42) {
            printf("error when recv msg\n");
            return;
        }

        P_ETH_HEAD eth = (void *)buffer;

        printf("dst mac %s\n", mactoa(eth->dst_mac));
        printf("src mac %s\n", mactoa(eth->src_mac));
        printf("type: %04x\n", ntohs(eth->type));


        struct arp_packet *arp;
        switch (ntohs(eth->type))
        {
            case 0x0800:
                ip_route(sock_fd, buffer, n_read);
                break;
            case 0x0806:
				//
				//  TODO Add arp handler here
				//
                arp = (void *)(buffer);
                if (arp->opt != 0x0200)
                    break;
                printf("ARP reply tpa %s tha %s\n",
                        iptoa(*(uint32_t *)(arp->spa)), mactoa(arp->sha));

                add_mac(*(uint32_t *)(arp->spa), arp->sha);
                resend_packet(*(uint32_t *)(arp->spa), arp->sha);
                break;
        }
        printf("============================================\n");
    }
}

int main()
{
    printf("hello, world\n");
    sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_fd < 0) {
        printf("error create new raw socket\n");
        return -1;
    }

    read_arp_cache();
    read_table();
	init_device_table();
    main_loop();
    return 0;
}
