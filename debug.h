#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdint.h>
// MAC address to ASCII
const char *mactoa(uint8_t *mac);
// IP address to ASCII
const char *iptoa(uint32_t ip);
#endif
