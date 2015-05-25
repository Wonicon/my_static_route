//
// Created by wonicon on 15-5-24.
//

#include <memory.h>
#include <unistd.h>
#include <assert.h>  // assert

#include <stdio.h>
#include <sys/socket.h>     // socket()
#include <sys/ioctl.h>
#include <arpa/inet.h>      // htons
#include <linux/ip.h>       // iphdr
#include <net/if.h>
#include <linux/if_ether.h>
#include <netinet/in.h>

#include "ip_forward.h"
#include "iptable.h"
#include "hdr.h"
#include "arptable.h"
#include "debug.h"
#include "simple_arp.h"

extern int sock_fd;
struct sockaddr a;
static struct sockaddr_ll socket_address;
static uint8_t src_mac[8];
static uint8_t src_ip[8];
static uint8_t dst_mac[8];

#define MAX_DEVICE 10
struct device_item
{
	char name[16];
	uint8_t mac[6];
	uint32_t ip;
	uint32_t mask;
} device_table[MAX_DEVICE];
int device_count = 0;

#include "arptable.h"
void init_device_table()
{	
	FILE *fd = fopen("device", "r");
	char mask[17];
	fscanf(fd, "%d", &device_count);
	printf("line %d\n", device_count);
	for (int i = 0; i < device_count; i++) {
		fscanf(fd, "%s", device_table[i].name);
		fscanf(fd, "%s", mask);
		device_table[i].mask = inet_addr(mask);
        read_interface(device_table[i].name, NULL, &device_table[i].ip,
                device_table[i].mac);
        add_mac(device_table[i].ip, device_table[i].mac);
	}
}

struct device_item *select_device(in_addr_t next_ip, int *out)
{
	for (int i = 0; i < device_count; i++) {
		uint32_t mask = device_table[i].mask;
		uint32_t ip = device_table[i].ip;
		if ((mask & next_ip) == (ip & mask)) {
            if (next_ip == ip) {
                printf("IP %s is the net card!\n", iptoa(next_ip));
                *out = 2;
            }
            else {
                printf("IP %s found device\n", iptoa(next_ip));
                *out = 1;
            }
			return &device_table[i];
		}
	}

	printf("No device found!\n");
    *out = 0;
	return NULL;
}

//
//  init the send addr
//
void init_addr(const char *device)
{
    /* Open RAW socket to send on */

    /* Get the index of the interface to send on */
    struct ifreq if_idx;
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, device, IFNAMSIZ - 1);
    if (ioctl(sock_fd, SIOCGIFINDEX, &if_idx) < 0) {
        perror("SIOCGIFINDEX");
    }

    /* Get the MAC address of the interface to send on */
    struct ifreq if_mac;
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, device, IFNAMSIZ - 1);
    if (ioctl(sock_fd, SIOCGIFHWADDR, &if_mac) < 0) {
        perror("SIOCGIFHWADDR");
    }

    /* Get the IP address of the interface to send on */
    struct ifreq if_ip;
    memset(&if_ip, 0, sizeof(struct ifreq));
    strncpy(if_ip.ifr_name, device, IFNAMSIZ - 1);
    if (ioctl(sock_fd, SIOCGIFADDR, &if_ip) < 0) {
        perror("SIOCGIFADDR");
    }

    /* Save the source MAC address */
    memcpy(src_mac, &if_mac.ifr_hwaddr.sa_data, 6);
    /* Save the source IP address */
    *(uint32_t *)src_ip = inet_addr(inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));

    /* Destination address */
    /* Index of the network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length */
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    memcpy(socket_address.sll_addr, dst_mac, 6);
}

#include "iptable.h"

//
//  routing
//
void ip_route(int sock, uint8_t *packet, int size)
{
	struct iphdr *ip = (void *)(packet + sizeof(ETH_HEAD));
    printf("src ip %s\n", iptoa(ip->saddr));
    printf("dst ip %s\n", iptoa(ip->daddr));

    int dsttype, srctype;
    struct device_item *dstdev = select_device(ip->daddr, &dsttype);
	select_device(ip->saddr, &srctype);
    
    uint8_t *mac;
    if (dsttype == 2 || srctype == 2) {
        printf("No need to route.\n");
        return;
    }

    uint32_t next_ip;
    if (dsttype == 0) {
        printf("Routing...\n");
        IPTE *next = next_hop(ip->daddr);
        if (next == NULL) {
            return;
        }
        next_ip = next->src;
        dstdev = select_device(next_ip, &dsttype);
   }
    else {
        printf("Directly forwarding\n");
        next_ip = ip->daddr;
    }

    mac = get_mac(next_ip);
    if (mac == NULL) {
        //  TODO ARP
        printf("Send arp request...\n");
        make_arp(next_ip, dstdev->ip, dstdev->mac, dstdev->name);
        return;
    }
 
    forward(sock, dstdev->name, mac, packet, size);
}

//
//  forward a packet,
//  need the device name, and the destination mac address
//  this mac address may belongs to the destination ip or the next hop ip
//
void forward(int sock, const char *device, uint8_t *mac, uint8_t *packet, int n)
{
    P_ETH_HEAD eth = (void *)packet;
	memcpy(dst_mac, mac, 6);
	init_addr(device);
	memcpy(eth->dst_mac, dst_mac, 6);
	printf("Sending...\n");
	sendto(sock_fd, packet, n, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll));
}
