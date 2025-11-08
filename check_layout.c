#include <stdio.h>
#include <stdint.h>

unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t unused2[64];
char * secret = "abcdefghijklmnopq.";

int main() {
    printf("array1: %p\n", (void*)array1);
    printf("unused2: %p\n", (void*)unused2);
    printf("secret: %p\n", (void*)secret);
    printf("\nOffset secret - array1: %ld\n", (char*)secret - (char*)array1);
    printf("Offset unused2 - array1: %ld\n", (char*)unused2 - (char*)array1);

    size_t malicious_x = (size_t)(secret - (char*)array1);
    printf("\nmalicious_x (size_t): %zu (0x%zx)\n", malicious_x, malicious_x);

    return 0;
}
