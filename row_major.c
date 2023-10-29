#include "Performance.h"
#include "ApplicationMemory.h"
#include <stdio.h>

#define N 9

void reportError(Address addr, int expected, int found)
{
    char msg[200];
    sprintf(msg, "ERROR: bad value @ %08x (%d)  value: %d  expected: %d\n", addr, addr, found, expected);
    printf("%s", msg);
    perfNote(msg);
}

int main(int argc, char**argv)
{
    // fill a 9x9 matrix in row major order

    int value;
    int baseAddr = 0x4000;
    clearPerformanceCounters();
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            writeMemory(baseAddr + (i * N * 4) + (j * 4), i * N + j);
        }
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            value = readMemory(baseAddr + (i * N * 4) + (j * 4));
            if (value != i * N + j) {
                reportError(baseAddr + (i * N * 4) + (j * 4), i * N + j, value);
            }
        }
    }
    flushMemory();
    struct PerformanceCounters pc;
    getPerformanceCounters(&pc);
    printPerformanceInfo(&pc);

    printf("row major order done\n");
}
