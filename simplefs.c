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
    strcpy(fdb.fcb.name, "/0");       //need to use strcpy, without error
    fdb.header.previous_block = -1;
    fdb.header.next_block = -1;       //in the future it could be have a next block, now it hasn't
    fdb.header.block_in_file = 0;     //repeatedly position 0
    
    //no need to clear bitmap block because in DiskDriver_init i've just cleared

    if(DiskDriver_writeBlock(fs ->disk, &fdb, 0) == -1){
        printf("Error in writing disk at position 0 (for FirstDirectoryBlock) in SimpleFS_format\n");
        return;
    }
    
}

FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename){
    
    if(d == NULL || filename == NULL){
        printf("DirectoryHandle or filename are NULL\n");
        return NULL;
    }

    FirstFileBlock ffb;         //using for reading from disk...
    FirstDirectoryBlock* firstdirblock = d ->dcb;
    DiskDriver* disk = d ->sfs ->disk;

    if(DiskDriver_getFreeBlock(d ->sfs ->disk, 0) == -1){
        printf("No space on disk remained to create new file\n");
        return NULL;
    }

    if(firstdirblock ->num_entries > 0){
        
        //taking FirstDirectoryBlock (and after directory block using blockheader) i can try to find if there is just the same file i want to create
        int i;
        
        for(i = 0; i < firstdirblock ->num_entries; i++){
            
            if(firstdirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(disk, &ffb, firstdirblock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                if(strcmp(ffb.fcb.name, filename) == 0){
                    printf("File already exists, change filename please\n");
                    return NULL;
                }
            }
        }

        int nextdir = firstdirblock ->header.next_block;
        DirectoryBlock* dirblock;

        while(1){

            if(nextdir == -1)
                break;

            if(DiskDriver_readBlock(disk, dirblock, nextdir) == -1){
                printf("impossible to read directories after firstdirectory\n");
                return NULL;
            }
            
            if(dirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(disk, &ffb, dirblock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                if(strcmp(ffb.fcb.name, filename) == 0){
                    printf("File already exists, change filename please\n");
                    return NULL;
                }
            }

            nextdir = dirblock ->header.next_block;
        }
    }
        
    //if arrived here, there aren't any file that has the same filename, so i can create it

    int free_block_to_mem;
    if((free_block_to_mem = DiskDriver_getFreeBlock(disk, 0)) == -1){
        printf("No free blocks in the disk\n");
        return NULL;
    }

    FirstFileBlock newFile = { 1 };
    newFile.header.block_in_file = 0;
    newFile.header.next_block = -1;
    newFile.header.previous_block = -1;
    newFile.fcb.is_dir = 0;
    strcpy(newFile.fcb.name, filename);
    newFile.fcb.block_in_disk = free_block_to_mem;
    newFile.fcb.directory_block = firstdirblock ->fcb.block_in_disk;    //because starting from FirstDirectoryBlock

    if(DiskDriver_writeBlock(disk, &newFile, free_block_to_mem) == -1){
        printf("Cannot write new file on disk\n");
        return NULL;
    }

    if(firstdirblock ->num_entries < (BLOCK_SIZE - sizeof(BlockHeader) -sizeof(FileControlBlock) -sizeof(int)) / sizeof(int)){ //if it's ok i can put new file block in FirstDirectoryBlock
        
        int i;
        for(i = 0; i < firstdirblock ->num_entries; i++){
            if(firstdirblock ->file_blocks[i] == 0){
                firstdirblock ->num_entries++;
                firstdirblock ->file_blocks[i] = i;
                break;
            }
        }

    }else{
        
        int nextdir = firstdirblock ->header.next_block;
        DirectoryBlock* dirblock;

        while(1){

            if(nextdir == -1)
                break;

            if(DiskDriver_readBlock(disk, dirblock, nextdir) == -1){
                printf("impossible to read directory after firstdirectory\n");
                return NULL;
            }

            int i;
            for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int); i++){
                if(dirblock ->file_blocks[i] == 0){  //if block is empty is useless to read it
                    firstdirblock ->num_entries++;
                    dirblock ->file_blocks[i] = &newFile;
                    break;
                }
            }

            nextdir = dirblock ->header.next_block;
        }

    }

    FileHandle* filehandle = (FileHandle*)malloc(sizeof(FileHandle));
    filehandle ->directory = firstdirblock;
    filehandle ->fcb = &newFile;
    filehandle ->pos_in_file = 0;
    filehandle ->sfs = d ->sfs;

    return filehandle;
    
}