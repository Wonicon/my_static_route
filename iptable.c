//
// Created by Wang Huizhe on 15-5-24.
//

#include "iptable.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#define TABLE_SIZE 1024

static IPTE table[TABLE_SIZE];
static int n_table;

void read_table()
{
    memset(table, 0, sizeof(table));
    FILE *fd = fopen("ip_table.txt", "r");
    uint8_t dst[4], src[4], mask[4];
    char buffer[1024];
    char *p;
    fgets(buffer, 1024, fd);
    sscanf(buffer, "%d", &n_table);
    printf("%d entries\n", n_table);
    for (int k = 0; k < n_table; k++) {
        fgets(buffer, 1024, fd);
        p = buffer;
        for (int i = 0; i < 4; i++) {
            dst[i] = (uint8_t) strtol(p, &p, 10);
            p++;
        }

        for (int i = 0; i < 4; i++) {
            mask[i] = (uint8_t)strtol(p, &p, 10);
            p++;
        }

        for (int i = 0; i < 4; i++) {
            src[i] = (uint8_t)strtol(p, &p, 10);
            p++;
        }

        printf("%s", p);
        sscanf(p, "%s", table[k].dev);

        for (int i = 0; i < 4; i++) {
            printf("%u ", dst[i]);
        }
        for (int i = 0; i < 4; i++) {
            printf("%u ", mask[i]);
        }
        printf("to ");
        for (int i = 0; i < 4; i++) {
            printf("%u ", src[i]);
        }

        table[k].dst = *(uint32_t *)dst;
        table[k].src = *(uint32_t *)src;
        table[k].mask = *(uint32_t *)mask;

        printf("\n");
    }
}

//
// Display ip addr
//
void print_ip(uint32_t addr)
{
    uint8_t *p;
    p = (void *)&addr;
    printf("%d:%d:%d:%d ", p[0], p[1], p[2], p[3]);
}

//
//  Search for the next src ip addr according to the dst ip addr
//
IPTE *next_hop(uint32_t dst)
{
    printf("In next_hop dst addr");
    print_ip(dst);
    printf("\n");
    uint32_t mask;
    for (int i = 0; i < n_table; i++) {
        mask = table[i].mask;
        if ((mask & table[i].dst) == (mask & dst)) {
			printf("hit\n");
            return table + i;
        }
    }

    // if not found
	printf("ip table miss\n");
    return NULL;
}
