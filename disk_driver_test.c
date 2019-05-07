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
    printf("bitmap_data: ");

    int i;

    for(i = 0; i < diskdriver ->header ->bitmap_blocks; i++){
        printf("%d", diskdriver ->bitmap_data[i]);
    }
    printf("\n");

    if(!DiskDriver_flush(diskdriver))
        printf("Flushing with success\n");

    //Started test from here...

    free(diskdriver);

    return 0;
}