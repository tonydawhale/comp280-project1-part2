#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "DRAM_Cache.h"
#include "Performance.h"

#define NUM_SETS 4
#define NUM_LINES 2 

struct CacheLineData {
    CacheLine data;
    int tag:9;
    bool valid;
    bool dirty;
    int timestamp;
};

int clock = 0;

struct CacheLineData DRAMCache[NUM_SETS][NUM_LINES];

void initCache() {
    for (int i = 0; i < NUM_SETS; i++) {
        for (int j = 0; j < NUM_LINES; j++) {
            memset(DRAMCache[i][j].data, 0, sizeof(CacheLine));
            DRAMCache[i][j].tag = 0;
            DRAMCache[i][j].valid = false;
            DRAMCache[i][j].dirty = false;
            DRAMCache[i][j].timestamp = 0;
        }
    }
}
int readWithCache(Address addr) {
    if ((addr & 0x1f) % 4 == 0) {
        int value, line;
        int set = (addr >> 5) & 0x3;
        int addressTag = (addr >> 7) & 0x1ff;
        bool hit = false;
        bool compulsory = false;
        for (int i = 0; i < NUM_LINES; i++) { // Check if there is a hit and store the line
            if (DRAMCache[set][i].tag == addressTag && DRAMCache[set][i].valid) {
                hit = true;
                line = i;
                break;
            }
        }
        if (hit) {
            memcpy(&value, &DRAMCache[set][line].data, 4);
            DRAMCache[set][line].timestamp = clock++;
            perfCacheHit(addr, set, value);
            return value;
        } else {
            for (int i = 0; i < NUM_LINES; i++) { // Check if there is a compulsory miss and store the line
                if (!DRAMCache[set][i].valid) {
                    compulsory = true;
                    line = i;
                    break;
                }
            }
            if (!compulsory) {
                line = DRAMCache[set][1].timestamp < DRAMCache[set][0].timestamp ? 1 : 0;
            }

            if (!compulsory && DRAMCache[set][line].dirty) {
                writeDramCacheLine(addr, DRAMCache[set][line].data);
                perfDramCacheLineWrite(addr, DRAMCache[set][line].data);
            }

            readDramCacheLine(addr & ~0x1f, DRAMCache[set][line].data);
            DRAMCache[set][line].tag = addressTag;
            DRAMCache[set][line].valid = true;
            DRAMCache[set][line].dirty = false;
            DRAMCache[set][line].timestamp = clock++;
            memcpy(&value, &DRAMCache[set][line].data, 4);
            perfCacheMiss(addr, set, value, compulsory);
            return value;
        }
    }
    else {
        printf("Error: Address is not aligned to 4 bytes\n");
        return 0;
    }
}
void writeWithCache(Address addr, int value) {
    if ((addr & 0x1f) % 4 == 0) {
        int set = (addr >> 5) & 0x3;
        int addressTag = (addr >> 7) & 0x1ff;
        int line;
        bool hit = false;
        bool compulsory = false;
        for (int i = 0; i < NUM_LINES; i++) { // Check if there is a hit and store the line
            if (DRAMCache[set][i].tag == addressTag && DRAMCache[set][i].valid) {
                hit = true;
                line = i;
                break;
            }
        }
        if (hit) { // If there is a hit
            // DRAMCache[set][line].data[(addr & 0x1f)] &= 0x00000000; // clears 4 bytes where we want to store value(4 bytes)
            // DRAMCache[set][line].data[(addr & 0x1f)] >>= (addr & 0x1f); // shifts value to the right position
            // DRAMCache[set][line].data[(addr & 0x1f)] |= value; // stores value into CacheLine data
            memset(&DRAMCache[set][line].data, value, 4);
            DRAMCache[set][line].dirty = true;
            DRAMCache[set][line].timestamp = clock++;
            perfCacheHit(addr, set, value);
        } else {
            for (int i = 0; i < NUM_LINES; i++) { // Check if there is a compulsory miss and store the line
                if (!DRAMCache[set][i].valid) {
                    compulsory = true;
                    line = i;
                    break;
                }
            }
            
            if (!compulsory) {
                line = DRAMCache[set][1].timestamp < DRAMCache[set][0].timestamp ? 1 : 0;

                if (DRAMCache[set][line].dirty) {
                    writeDramCacheLine(addr, DRAMCache[set][line].data);
                    perfDramCacheLineWrite(addr, DRAMCache[set][line].data);
                }

                readDramCacheLine(addr, DRAMCache[set][line].data);
                memset(&DRAMCache[set][line].data, value, 4);
                DRAMCache[set][line].dirty = true;
                DRAMCache[set][line].timestamp = clock++;
                perfCacheHit(addr, set, value);
            } else {
                readDramCacheLine(addr, DRAMCache[set][line].data);
                // DRAMCache[set][line].data[(addr & 0x1f)] &= 0x00000000; // clears 4 bytes where we want to store value(4 bytes)
                // DRAMCache[set][line].data[(addr & 0x1f)] >>= (addr & 0x1f); // shifts value to the right position
                // DRAMCache[set][line].data[(addr & 0x1f)] |= value; // stores value into CacheLine data
                memset(&DRAMCache[set][line].data, value, 4);
                DRAMCache[set][line].dirty = false;
                DRAMCache[set][line].timestamp = clock++;
                perfCacheHit(addr, set, value);
            }
            perfCacheMiss(addr, set, value, compulsory);
        }
    }
    else {
        printf("Error: Address is not aligned to 4 bytes\n");
    }
}
void flushCache() {
    for (int i = 0; i < NUM_SETS; i++) {
        for (int j = 0; j < NUM_LINES; j++) {
            if (DRAMCache[i][j].dirty) {
                int addr = (DRAMCache[i][j].tag >> 5) & 0x7ff;
                DRAMCache[i][j].dirty = false;
                writeDramCacheLine(addr, DRAMCache[i][j].data);
            }
        }
    }
    perfCacheFlush();
}