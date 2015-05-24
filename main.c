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
#define BUFFER_MAX 2048

int sock_fd;

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

        for (int i = 0; i < 6; i++) {
            printf("%02x ", eth->dst_mac[i]);
        }
        printf("\n");
        for (int i = 0; i < 6; i++) {
            printf("%02x ", eth->src_mac[i]);
        }
        printf("\ntype: %04x\n", ntohs(eth->type));


        P_ARP_DATAGRAM arp;
        switch (ntohs(eth->type))
        {
            case 0x0800:
				//
				//  TODO Complete routing code here
				//
                ip_route(sock_fd, buffer, n_read);
                break;
            case 0x0806:
				//
				//  TODO Add arp handler here
				//
                arp = (void *)(buffer + sizeof(*eth));
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

	init_device_table();
    read_table();
    read_arp_cache();
    main_loop();
    return 0;
}
