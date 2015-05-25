#include "simple_arp.h"

#include <unistd.h>  // close()
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <net/if.h>

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

    s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
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
    strncpy_s(addr.sa_data, interface, sizeof(addr.sa_data));
    if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0) {
        perror("Send arp packet failed");
    }

ret:
    close(s);
    return 1;
}
