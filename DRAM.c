#include <stdio.h>
#include <string.h>

#include "DRAM.h"
#include "Performance.h"

#define DRAM_LENGTH 49152

unsigned char DRAM[DRAM_LENGTH];

int readDram(Address addr) {
    if (addr % 4 == 0) {
        int res;
        memcpy(&res, &DRAM[addr], 4);
        perfDramRead(addr, res);
        return res;
    } else {
        printf("The address offset is incorrect.");
        return 0;
    }
}

void writeDram(Address addr, int value) {
    if (addr % 4 == 0) {
        memcpy(&DRAM[addr], &value, 4);
        perfDramWrite(addr, value);
    } else {
        printf("The address offset is incorrect.");
    }
}

void readDramCacheLine(Address addr, CacheLine line) {
    if (addr % 4 == 0) {
        memcpy(line, &DRAM[addr], 32);
        perfDramCacheLineRead(addr, line);
    } else {
        printf("The address offset is incorrect.");
    }
}

void writeDramCacheLine(Address addr, CacheLine line) {
    if (addr % 4 == 0) {
        memcpy(&DRAM[addr], line, 32);
        perfDramCacheLineWrite(addr, line);
    } else {
        printf("The address offset is incorrect.");
    }
}
