#include <stdio.h>

const char *
mactoa(uint8_t *mac)  // mac address to ascii
{
    static char s[20];
    sprintf(s,
            "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2],
            mac[3], mac[4], mac[5]);
    return s;
}

const char *
iptoa(uint32_t *ip)  // ip address to ascii
{
    static char s[20];
    uint8_t *p = (void *)&ip;
    sprintf(s,
            "%d.%d.%d.%d",
            p[3], p[2], p[1], p[0]);
    return s;
}

