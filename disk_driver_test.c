#include <stdio.h>
#include <stdlib.h>
#include "disk_driver.h"

int main(int argc, char* argv[]){
    
    DiskDriver* diskdriver = (DiskDriver*)malloc(sizeof(DiskDriver));
    const char* filename = "./disk.txt";

    DiskDriver_init(diskdriver, filename, 16);

    printf("bitmap_blocks: %d\n", diskdriver ->header ->bitmap_blocks);
    printf("bitmap_entries: %d\n", diskdriver ->header ->bitmap_entries);
    printf("first_free_block :%d\n", diskdriver ->header ->first_free_block);
    printf("free_blocks: %d\n", diskdriver ->header ->free_blocks);
    printf("num_blocks: %d\n", diskdriver ->header ->num_blocks);

    int i = 0;

    BitMap bmap;
    bmap.entries = diskdriver ->bitmap_data;
    bmap.num_bits = diskdriver ->header ->bitmap_blocks;

    int a = bmap.entries[0];
    
    printf("primo blocco: ");
    for (i = 7; i >= 0; i--) {
        printf("%d", !!((a >> i) & 0x01));
    }
    printf("\n");

    a = bmap.entries[1];
    printf("secondo blocco: ");
    for (i = 7; i >= 0; i--) {
        printf("%d", !!((a >> i) & 0x01));
    }
    printf("\n");
    
    free(diskdriver);

    return 0;
}