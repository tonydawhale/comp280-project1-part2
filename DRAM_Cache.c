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

int getTag(Address addr) {
    return (addr >> 7) & 0x1ff;
}
int getSet(Address addr) {
    return (addr >> 5) & 0x3;
}
int getNewTag(int set, int line) {
    return ((DRAMCache[set][line].tag << 7) | (set << 5)) & 0xFFFF;
}
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
        int set = getSet(addr);
        int addressTag = getTag(addr);
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
            perfCacheHit(addr, set, line);
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
                int newAddr = getNewTag(set, line);
                writeDramCacheLine(newAddr, DRAMCache[set][line].data);
                DRAMCache[set][line].dirty = false;
            }

            readDramCacheLine(addr & ~0x1f, DRAMCache[set][line].data);
            DRAMCache[set][line].tag = addressTag;
            DRAMCache[set][line].valid = true;
            DRAMCache[set][line].dirty = false;
            DRAMCache[set][line].timestamp = clock++;
            memcpy(&value, &DRAMCache[set][line].data, 4);
            perfCacheMiss(addr, set, line, compulsory);
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
        int set = getSet(addr);
        int addressTag = getTag(addr);
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
            memcpy(&DRAMCache[set][line].data, &value, 4);
            DRAMCache[set][line].tag = addressTag;
            DRAMCache[set][line].dirty = true;
            DRAMCache[set][line].timestamp = clock++;
            perfCacheHit(addr, set, line);
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

                if (DRAMCache[set][line].dirty) { // Check if the data is dirty
                    int newAddr = getNewTag(set, line);
                    writeDramCacheLine(newAddr, DRAMCache[set][line].data);
                    DRAMCache[set][line].dirty = false;
                }
            }
            readDramCacheLine(addr & ~0x1f, DRAMCache[set][line].data);
            memcpy(&DRAMCache[set][line].data, &value, 4);
            DRAMCache[set][line].tag = addressTag;
            DRAMCache[set][line].dirty = true;
            DRAMCache[set][line].valid = true;
            DRAMCache[set][line].timestamp = clock++;
            perfCacheMiss(addr, set, line, compulsory);
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
                int addr = getNewTag(i, j);
                DRAMCache[i][j].dirty = false;
                writeDramCacheLine(addr, DRAMCache[i][j].data);
            }
        }
    }
    perfCacheFlush();
}