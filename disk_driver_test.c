#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    char fb1[BLOCK_SIZE - sizeof(BlockHeader)];             //must use a temp char[] to memorize the data and after use strcpy to copy them in flbt ->data
    for(i = 0; i < BLOCK_SIZE - sizeof(BlockHeader); i++)   //strcpy copy in a correct way the data, without strcpy i got an error: "Address 0x522d6e0 is 0 bytes after a block of size 512 alloc'd"
        fb1[i] = '7';
    fb1[BLOCK_SIZE - sizeof(BlockHeader) - 1] = '\0';
    strcpy(flb1.data, fb1);
    
    
    FileBlock flb2;
    flb2.header = blockheader;
    char fb2[BLOCK_SIZE - sizeof(BlockHeader)];
    for(i = 0; i < BLOCK_SIZE - sizeof(BlockHeader); i++)
        fb2[i] = '1';
    fb2[BLOCK_SIZE - sizeof(BlockHeader) - 1] = '\0';
    strcpy(flb2.data, fb2);
    
    FileBlock flb3;
    flb3.header = blockheader;
    char fb3[BLOCK_SIZE - sizeof(BlockHeader)];
    for(i = 0; i < BLOCK_SIZE - sizeof(BlockHeader); i++)
        fb3[i] = '5';
    fb3[BLOCK_SIZE - sizeof(BlockHeader) - 1] = '\0';
    strcpy(flb3.data, fb3);

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

    printf("I'm writing third FileBlock(flb3) in disk in position 2\n");
    if(DiskDriver_writeBlock(diskdriver, &flb3, 2) == -1){
        printf("Error in writing block 2 of flb3\n");
        return -1;
    }
    DiskDriver_flush(diskdriver);
    printf("Bitmap in posizione 2: %d\n", diskdriver ->bitmap_data[2]);
    printf("free_blocks: %d\n", diskdriver ->header ->free_blocks);
    printf("first_free_block :%d\n", diskdriver ->header ->first_free_block);
    printf("Success\n\n\n");

    printf("I'm writing third FileBlock(flb3) in disk in position 0\n");
    if(DiskDriver_writeBlock(diskdriver, &flb3, 0) == -1){
        printf("Error in writing block 0 of flb3\n");
        printf("Success\n\n\n");

    }
    DiskDriver_flush(diskdriver);

    //Trying to read something on the disk
    //char arr1[BLOCK_SIZE];    cannot use them, must using a struct FileBlock 
    //char arr2[BLOCK_SIZE];
    //char arr3[BLOCK_SIZE];

    FileBlock* flbt = (FileBlock*) malloc(sizeof(FileBlock));
    flbt ->header = blockheader;
    
    printf("leggo in posizione 0\n");
    if(DiskDriver_readBlock(diskdriver, flbt, 0) != -1){
        printf("%s\n", flbt ->data);
        printf("Success\n\n\n");
    }
    else{
        printf("free space or incorrect position\n");
        printf("Insuccess\n\n\n");    }

    
    printf("leggo in posizione 1\n");
    if(DiskDriver_readBlock(diskdriver, flbt, 1) != -1){
        printf("Dati: %s\n", flbt ->data);
        printf("Success\n\n\n");
    }
    else{
        printf("free space or incorrect position\n");
        printf("Insuccess\n\n\n");
    }
    
    printf("leggo in posizione 2\n");
    if(DiskDriver_readBlock(diskdriver, flbt, 2) != -1){
        printf("Dati: %s\n", flbt ->data);
        printf("Success\n\n\n");
    }
    else{
        printf("free space or incorrect position\n");
        printf("Insuccess\n\n\n");
    }
    
    printf("leggo in posizione 3\n");
    if(DiskDriver_readBlock(diskdriver, flbt, 3) != -1){
        printf("Dati: %s\n", flbt ->data);
        printf("Insuccess\n\n\n");
    }
    else{
        printf("free space or incorrect position\n");
        printf("Success\n\n\n");
    }
    
    //Try getFreeBlock function and FreeBLock
    int ret;

    printf("Provo a ottenere un blocco libero(sono finiti..)\n");
    if((ret = DiskDriver_getFreeBlock(diskdriver, 0)) != -1){
        printf("free space at position: %d\n", ret);
        printf("Insuccess\n\n\n");
    }
    else{
        printf("Sorry but there aren't free space anymore\n");
        printf("Success\n\n\n");
    }

    printf("Libero memoria in posizione 0\n");
    if((ret = DiskDriver_freeBlock(diskdriver, 0)) != -1){
        printf("freed space at position: %d\n", ret);
        printf("Success\n\n\n");
    }
    else{
        printf("Cannot free space at position 0\n");
        printf("Insuccess\n\n\n");
    }

    printf("Riprovo a ottenere un blocco libero\n");
    if((ret = DiskDriver_getFreeBlock(diskdriver, 0)) != -1){
        printf("free space at position: %d\n", ret);
        printf("Success\n\n\n");
    }
    else{
        printf("Sorry but there aren't free space anymore\n");
        printf("Insuccess\n\n\n");
    }
    
    DiskDriver_flush(diskdriver);
    free(flbt);
    free(diskdriver);

    return 0;
}