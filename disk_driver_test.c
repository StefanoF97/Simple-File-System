#include <stdio.h>
#include <stdlib.h>
#include "disk_driver.h"
#include "simplefs.h"

int main(int argc, char* argv[]){
    
    DiskDriver* diskdriver = (DiskDriver*)malloc(sizeof(DiskDriver));
    const char* filename = "./disk.txt";

    DiskDriver_init(diskdriver, filename, 3);

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
        printf("Flushing with success\n\n\n");

    //Started test from here...

    //I'm going to create 3 file block when i can write som data, before i need to alloc the header(in simplefs it will explain the chain...)
    BlockHeader blockheader;
    blockheader.next_block = 1;
    blockheader.previous_block = 1;
    blockheader.block_in_file = 1;

    FileBlock flb1;
    flb1.header = blockheader;
    
    for(i = 0; i < BLOCK_SIZE - sizeof(BlockHeader); i++)
        flb1.data[i] = 7;
    
    FileBlock flb2;
    flb2.header = blockheader;
    for(i = 0; i < BLOCK_SIZE - sizeof(BlockHeader); i++)
        flb2.data[i] = 1;
    
    FileBlock flb3;
    flb3.header = blockheader;
    for(i = 0; i < BLOCK_SIZE - sizeof(BlockHeader); i++)
        flb3.data[i] = 1;

    //Trying to write something on the disk starting from fileblock
    printf("I'm writing first FileBlock(flb1) in disk in position 0\n");
    if(DiskDriver_writeBlock(diskdriver, &flb1, 0) == -1){
        printf("Error in writing block 0 of flb1\n");
        return -1;
    }
    DiskDriver_flush(diskdriver);
    printf("Bitmap in posizione 0: %d\n", diskdriver ->bitmap_data[0]);
    printf("free_blocks: %d\n", diskdriver ->header ->free_blocks);
    printf("first_free_block :%d\n", diskdriver ->header ->first_free_block);
    printf("Success\n\n\n");

    printf("I'm writing second FileBlock(flb2) in disk in position 0\n");
    if(DiskDriver_writeBlock(diskdriver, &flb2, 0) == -1){
        printf("Error in writing block 0 of flb1\n");
        printf("Success\n\n\n");
    }
    DiskDriver_flush(diskdriver);

    printf("I'm writing second FileBlock(flb2) in disk in position 1\n");
    if(DiskDriver_writeBlock(diskdriver, &flb2, 1) == -1){
        printf("Error in writing block 1 of flb2\n");
        return -1;
    }
    DiskDriver_flush(diskdriver);
    printf("Bitmap in posizione 1: %d\n", diskdriver ->bitmap_data[1]);
    printf("free_blocks: %d\n", diskdriver ->header ->free_blocks);
    printf("first_free_block :%d\n", diskdriver ->header ->first_free_block);
    printf("Success\n\n\n");

    free(diskdriver);

    return 0;
}