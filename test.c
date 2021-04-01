#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include "tpool.h"


// uint32_t lfsr2(uint32_t poly)
// {
//     uint32_t start_state = 0xACE1u;  /* Any nonzero start state will work. */
//     uint32_t lfsr = start_state;
//     uint32_t period = 0;

//     do
//     {"
//         uint32_t msb = lfsr & (1 << 23);

//         lfsr <<= 1;                          /* Shift register */
//         lfsr &= 0xFFFFFF;
//         if (msb)                             /* If the output bit is 1, */
//             lfsr ^= poly;                 /*  apply toggle mask. */
//         ++period;
//     }
//     while (lfsr != start_state);

//     return period;
// }

uint64_t galois_lfsr_find_period(uint64_t poly, uint64_t order)
{
    uint64_t start_state = 0xACE1u;  /* Any nonzero start state will work. */
    uint64_t lfsr = start_state;
    uint64_t period = 0;
    uint64_t msb_mask = 1UL << (order - 1UL);
    uint64_t state_mask = (1UL << order) - 1UL;

    do
    {
        uint64_t msb = lfsr & msb_mask;

        lfsr <<= 1;                          /* Shift register */
        lfsr &= state_mask;
        if (msb)                             /* If the output bit is 1, */
            lfsr ^= poly;                 /*  apply toggle mask. */
        ++period;
    }
    while (lfsr != start_state);

    return period;
}


void basic_find_maximal(uint64_t nbits) {
    uint64_t supposed_max = (1UL << nbits) - 1;
    uint64_t period = 0;
    uint64_t poly = 0;
    for(poly=1; poly<256; poly += 2) {
        period = galois_lfsr_find_period(poly, nbits);
        // printf("poly: 0x%02lx, period: 0x%lx\n", poly, period);
        if (period == supposed_max) {
            printf("poly: (%ld) 0x%02lx\n", poly, poly);
            // break;
        }
    }
}
// 0x1b is maximal for 24 bits
// 0xaf is maximal for 32 bits
// 0x39 (also 0xd7) is maximal for 40 bits

// The following were found by the Python script
/* 
48: maximals = [183]
56: maximals = [149]
64: maximals = [27, 29, 245]
72: maximals = [95]
80: maximals = [175]
88: maximals = []
96: maximals = [221]
104: maximals = []
112: maximals = []
120: maximals = [231]
128: maximals = [135]
136: maximals = []
144: maximals = [149]
152: maximals = [77]
160: maximals = [45, 57]
168: maximals = []
176: maximals = [189]
184: maximals = []
192: maximals = []
200: maximals = [45]
208: maximals = []
216: maximals = [139, 189]
224: maximals = []
232: maximals = []
240: maximals = []
248: maximals = []
256: maximals = []
*/



uint64_t periods[128] = {0};


#define NBITS_PER_WORKER 2

void worker(void * arg) {
    uint64_t nbits = 48;
    uint64_t startval = (uint64_t)arg;
    uint64_t supposed_max = (1UL << nbits) - 1;
    uint64_t period = 0;
    uint64_t poly = 0;
    for(uint64_t i=1; i<(1<<NBITS_PER_WORKER); i += 2) {
        poly = startval + i;
        period = galois_lfsr_find_period(poly, nbits);
        if (period == supposed_max) {
            printf("MAXIMAL poly: 0x%02lx, period: 0x%lx\n", poly, period);
            break;
        }
        printf("poly: 0x%02lx, period: 0x%lx\n", poly, period);
    }
    periods[poly>>1] = period;
}

void threaded_find_maximal() {
    tpool_t * pool;
    int nprocs = get_nprocs();
    printf("Starting %d workers\n", nprocs);
    pool = tpool_create(nprocs);
    for (int i=0; i<(1<<(8-NBITS_PER_WORKER)); i++) {
        uint64_t startval = i<<NBITS_PER_WORKER;
        tpool_add_work(pool, worker, (void *)startval);
    }
    tpool_wait(pool);
    tpool_destroy(pool);

}


uint64_t galois_lfsr_print(uint64_t poly, uint64_t order, uint64_t start_state, uint64_t steps)
{
    uint64_t lfsr = start_state;
    uint64_t period = 0;
    uint64_t msb_mask = 1UL << (order - 1UL);
    uint64_t state_mask = (1UL << order) - 1UL;

    for (uint64_t i=0; i<steps; i++)
    {
        uint64_t msb = lfsr & msb_mask;

        lfsr <<= 1;                          /* Shift register */
        lfsr &= state_mask;
        if (msb) {
            printf("1, ");
            lfsr ^= poly;                 /*  apply toggle mask. */
        }
        else {
            printf("0, ");
        }
        ++period;
    }
    printf("\n0x%lX\n", lfsr);

    return period;
}


int main() {
    threaded_find_maximal();
    uint64_t max_period = 0;
    uint64_t poly = 0;
    for (uint64_t i=0; i < sizeof(periods)/sizeof(periods[0]); i++) {
        if (periods[i] > max_period) {
            max_period = periods[i];
            poly = (i<<1) + 1;
        }
    }
    printf("maximal poly: 0x%02lx, period: 0x%lx\n", poly, max_period);


    // galois_lfsr_print(0x2DUL, 16UL, 0xACE1UL, 10);

    // basic_find_maximal(32);
    return 0;
}
