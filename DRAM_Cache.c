#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "DRAM_Cache.h"
#include "Performance.h"

#define NUM_SETS 4
#define NUM_LINES 2 

struct CacheLineData {
    CacheLine data;
    int tag:8;
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
        for (int i = 0; i < NUM_LINES; i++) {
            if (DRAMCache[set][i].valid && !DRAMCache[set][i].dirty && DRAMCache[set][i].tag == addressTag) { //for non-dirty hit
                perfCacheHit(addr, set, value);
                line = i;
                break;
            } 
            else if(DRAMCache[set][i].dirty == true && DRAMCache[set][i].tag == addressTag) { //for dirty hit
                readDramCacheLine(addr, DRAMCache[set][i].data);
                perfCacheHit(addr, set, value);
                perfDramCacheLineRead(addr, DRAMCache[set][i].data);
                line = i;
                break;
            }
            else if (!DRAMCache[set][i].valid && DRAMCache[set][i].tag != addressTag) { //for compulsory miss
                readDramCacheLine(addr, DRAMCache[set][i].data);
                perfCacheMiss(addr, set, value, true);
                perfDramCacheLineRead(addr, DRAMCache[set][i].data);
                line = i;
            }
            else if (DRAMCache[set][i].valid && DRAMCache[set][i].tag != addressTag) { //for non-compulsory miss
                readDramCacheLine(addr, DRAMCache[set][i].data);
                perfCacheMiss(addr, set, value, false);
                perfDramCacheLineRead(addr, DRAMCache[set][i].data);
                line = i;
            }
        }
        value = DRAMCache[set][line].data[(addr & 0x1f)];
        DRAMCache[set][line].timestamp = clock++;
        return value;  
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
        for (int i = 0; i < NUM_LINES; i++) {
            if (DRAMCache[set][i].tag == addressTag) { //for hit(tag matches)
                DRAMCache[set][line].data[(addr & 0x1f)] &= 0x00000000; // clears 4 bytes where we want to store value(4 bytes)
                DRAMCache[set][line].data[(addr & 0x1f)] |= value; // stores value into CacheLine data
                DRAMCache[set][line].valid = true;
                DRAMCache[set][line].dirty = true;
                DRAMCache[set][line].timestamp = clock++; 
                perfCacheHit(addr, set, value);
            }
            else if (DRAMCache[set][i].tag != addressTag) { //for miss
                if (DRAMCache[set]) {
                    
                }

            }

            
        }
    }
    else {
        printf("Error: Address is not aligned to 4 bytes\n");
    }
}
void flushCache() {
    
    perfCacheFlush();
}