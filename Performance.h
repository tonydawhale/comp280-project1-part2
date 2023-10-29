#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "DRAM.h"
#include <stdbool.h>


struct PerformanceCounters {
    int dramReads;
    int dramWrites;
    int dramCacheLineReads;
    int dramCacheLineWrites;
    int cacheHits;
    int cacheMisses;
    int dramCost;
};

void clearPerformanceCounters();
void getPerformanceCounters(struct PerformanceCounters * pc);
void printPerformanceInfo(struct PerformanceCounters * pc);

// DRAM stats
void perfDramRead(Address addr,  int value);
void perfDramWrite(Address addr, int value);
void perfDramCacheLineRead(Address addr, CacheLine cl);
void perfDramCacheLineWrite(Address addr, CacheLine cl);

// cache stats
void perfCacheHit(Address addr, int set, int entry);
void perfCacheMiss(Address addr, int set, int entry, bool compulsory);
void perfCacheFlush();


void beginMemoryAccess(Address addr, bool isRead);
void endMemoryAccess(Address addr, int value);

void perfNote(const char * note);

void perfIndent();
void perfOutdent();

#endif
