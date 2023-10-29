#include "Performance.h"
#include "ApplicationMemory.h"
#include "DRAM_Cache.h"
#include <stdio.h>


void reportError(Address addr, int expected, int found)
{
    char msg[200];
    sprintf(msg, "ERROR: bad value @ %08x (%d)  value: %d  expected: %d\n", addr, addr, found, expected);
    printf("%s", msg);
    perfNote(msg);
}

void simpleTest()
{
    int base = 40 * 1024;
    int lineNumber = 5;
    int blockSize = 32;
    int set = lineNumber * blockSize * 2;
    perfNote("writing to 4 blocks of memory");
    for (int tag = 0; tag < 4; ++tag) {
        int addr = base + tag * blockSize * 20 + set; 
        writeMemory(addr, addr);
    }

    perfNote("reading other blocks to force the 2 remaining dirty blocks to be written");
    for (int tag = 4; tag < 7; ++tag) {
        int addr = base + tag * blockSize * 20 + set; 
        readMemory(addr);
    }

    perfNote("checking the original writes");
    for (int tag = 0; tag < 4; ++tag) {
        int addr = base + tag * blockSize * 20 + set; 
        int value = readMemory(addr);
        if (value != addr) {
           reportError(addr, addr, value);
        }
    }
}

void writeReadTest()
{
    int value;
    for (int i = 0; i < 32; ++i) {
        writeMemory(i * 16, i);
    }
    for (int i = 0; i < 32; ++i) {
        value = readMemory(i * 16);
        if (value != i) {
            reportError(i * 16, i, value);
        }
    }
    flushMemory();
}

int main(int argc, char**argv)
{
    // step through memory writing a value to the start and middle of each cache line.
    clearPerformanceCounters();
    initCache();
    simpleTest();
    writeReadTest();
    struct PerformanceCounters pc;
    getPerformanceCounters(&pc);
    printPerformanceInfo(&pc);
    clearPerformanceCounters();

    printf("done\n");
}
