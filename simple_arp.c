#include <unistd.h>  // close()
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sys/ioctl.h>
#include <linux/if.h>

const int const_one = 1;
int set_broadcast(int fd)
{
    return setsockopt(
            fd,
            SOL_SOCKET,
            SO_BROADCAST,
            &const_one,
            sizeof(const_one));
}

#include "simple_arp.h"
#include "debug.h"
char *strncpy_s(char *dst, const char *src, size_t size)
{
    if (size == 0)
        return dst;
    dst[--size] = '\0';
    return strncpy(dst, src, size);
}

int make_arp(
        uint32_t    target_ip,
        uint32_t    sender_ip,
        uint8_t    *sender_mac,
        const char *interface)
{
    //int timeout_ms;
    int s;
    //int rv = 1;  // no reply received yet
    struct sockaddr addr;  // for interface name
    struct arp_packet arp;

    s = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
    if (s == -1) {
        perror("raw_socket");
        goto ret;
    }

    if (set_broadcast(s) == -1) {
        perror("Cannot enable bcast on raw socket");
        goto ret;
    }

    /* send arp request */
    memset(&arp, 0, sizeof(arp));
    memset(arp.dmac, 0xff, 6);  // Dest mac addr
    memcpy(arp.smac, sender_mac, 6);  // Src mac addr
    arp.type = htons(ETH_P_ARP);  // protocol type (ETHERNET-ARP)
    arp.htype = htons(ARPHRD_ETHER);  // hardware type
    arp.ptype = htons(ETH_P_IP);  // protocol type
    arp.hlen = 6;  // hardware address length
    arp.plen = 4;  // protocol address length
    arp.opt = htons(ARPOP_REQUEST);  // ARP op code (request)
    memcpy(arp.sha, sender_mac, arp.hlen);  // source hardware address
    memcpy(arp.spa, &sender_ip, arp.plen);  // srouce protocol address
    memcpy(arp.tpa, &target_ip, arp.plen);  // target protocol address
    memset(arp.tha, 0, arp.hlen);  // target mac address remain unknown

    /* sockaddr */
    memset(&addr, 0, sizeof(addr));
    printf("Interface %s, ip %s, mac %s\n",
            interface, iptoa(sender_ip), mactoa(sender_mac));
    strncpy_s(addr.sa_data, interface, sizeof(addr.sa_data));
    if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0) {
        perror("Send arp packet failed");
    }

ret:
    close(s);
    return 1;
}

int read_interface(
        const char *interface,
        int *ifindex,
        uint32_t *addr,
        uint8_t *mac)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *ip;
    
    memset(&ifr, 0, sizeof(ifr));
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    ifr.ifr_addr.sa_family = AF_INET;
    strcpy(ifr.ifr_name, interface);
    if (addr) {
        if (ioctl(fd, SIOCGIFADDR, &ifr) != 0) {
            perror("ioctl");
            close(fd);
            return -1;
        }
        ip = (struct sockaddr_in *) &ifr.ifr_addr;
        *addr = ip->sin_addr.s_addr;
        printf("ip of %s = %s\n", interface, iptoa(*addr));
    }

    if (ifindex) {
        if (ioctl(fd, SIOCGIFINDEX, &ifr) != 0) {
            close(fd);
            return -1;
        }
        printf("adapter index %d", ifr.ifr_ifindex);
        *ifindex = ifr.ifr_ifindex;
    }

    if (mac) {
        if (ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
            close(fd);
            return -1;
        }
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
        printf("adapter hardware address %s\n", mactoa(mac));
    }

    close(fd);
    return 0;
}
