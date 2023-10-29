#include "ApplicationMemory.h"
#include "Performance.h"
#include "DRAM.h"
#include "DRAM_Cache.h"


int readMemory(Address addr)
{
    beginMemoryAccess(addr, true);
    int result;
    result = readWithCache(addr);
    endMemoryAccess(addr, result);
    return result;
}

void writeMemory(Address addr, int value)
{
    beginMemoryAccess(addr, false);
    writeWithCache(addr, value);
    endMemoryAccess(addr, value);
}

void flushMemory()
{
    flushCache();
}
