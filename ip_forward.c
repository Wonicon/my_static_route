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

extern int sock_fd;
struct sockaddr a;
static struct sockaddr_ll socket_address;
static uint8_t src_mac[8];
static uint8_t src_ip[8];
static uint8_t dst_mac[8];

#define MAX_DEVICE 10
struct device_item
{
	uint8_t name[16];
	uint8_t mac[6];
	uint32_t ip;
	uint32_t mask;
} device_table[MAX_DEVICE];
int device_count = 0;

void init_device_table()
{	
	FILE *fd = fopen("device", "r");
	char ipaddr[17];
	fscanf(fd, "%d", &device_count);
	printf("line %d\n", device_count);
	for (int i = 0; i < device_count; i++) {
		fscanf(fd, "%s", device_table[i].name);
		fscanf(fd, "%s", ipaddr);
		device_table[i].ip = inet_addr(ipaddr);
		printf("%s %s ", device_table[i].name, ipaddr);
		fscanf(fd, "%s", ipaddr);
		device_table[i].mask = inet_addr(ipaddr);
		printf("%08x\n", device_table[i].mask);
	}
}

const char *select_device(in_addr_t next_ip)
{
	for (int i = 0; i < device_count; i++) {
		uint32_t mask = device_table[i].mask;
		uint32_t ip = device_table[i].ip;
		if ((mask & next_ip) == (device_table[i].ip & mask)) {
			printf("device hit!\n");
			return device_table[i].name;
		}
	}

	printf("no device found!\n");
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
	in_addr_t next_ip = ip->daddr;
	const char *device = select_device(ip->daddr);
	if (device == NULL) {
		//
		//  TODO use route table to select a device and update next_ip
		//
		printf("routing...");
		IPTE *next = next_hop(ip->daddr);
		device = select_device(next->src);
		next_ip = next->src;
	}

	assert(device != NULL);

	if (next_ip == ip->daddr) {
		printf("This is an easy routing.\n");
	}
	
	uint8_t *mac = get_mac(next_ip);
	if (mac == NULL) {
		//
		//  TODO send an arp request, and store this packet
		//  Now just throw it
		//
		return;
	}
	
	forward(sock, device, mac, packet, size);
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
	printf("sending...\n");
	sendto(sock_fd, packet, n, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll));
}
