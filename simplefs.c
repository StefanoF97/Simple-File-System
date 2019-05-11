#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simplefs.h"
#include "disk_driver.h"
#include "bitmap.h"

DirectoryHandle* SimpleFS_init(SimpleFS* fs, DiskDriver* disk){

    if(fs == NULL || disk == NULL){
        printf("Disk or SimpleFS are NULL in SimpleFS_init\n");
        return NULL;
    }

    fs ->disk = disk;

    DirectoryHandle* dh = (DirectoryHandle*)malloc(sizeof(DirectoryHandle));
    dh ->sfs = fs;
    dh ->directory = NULL;
    dh ->pos_in_block = 0;

    FirstDirectoryBlock* fsb = (FirstDirectoryBlock*)malloc(sizeof(FirstDirectoryBlock));
    if(DiskDriver_readBlock(disk, fsb, 0) == -1){
        printf("Error in reading in block 0 for FirstDirectoryBlock\n");
        return NULL;
    }

    dh ->dcb = fsb;
    printf("Init of SimpleFS is a success\n");

    return fsb;
}

void SimpleFS_format(SimpleFS* fs){
    
    if(fs == NULL){
        printf("fs is NULL in SimpleFS_format\n");
        return;
    }

    FirstDirectoryBlock fdb = { 0 };  //set to 0 to initialize
    fdb.fcb.block_in_disk = 0;        //in position 0
    fdb.fcb.directory_block = -1;     //top level diectory, like a "root", no parent directory
    fdb.fcb.is_dir = 1;
    strcpy(fdb.fcb.name, "\0");       //need to use strcpy, without error
    fdb.header.previous_block = -1;
    fdb.header.next_block = -1;       //in the future it could be have a next block, now it hasn't
    fdb.header.block_in_file = 0;     //repeatedly position 0
    fdb.num_entries = fs ->disk ->header ->bitmap_entries;

    //no need to clear bitmap block because in DiskDriver_init i've just cleared

    if(DiskDriver_writeBlock(fs ->disk, &fdb, 0) == -1){
        printf("Error in writing disk at position 0 in SimpleFS_format\n");
        return;
    }
    
}