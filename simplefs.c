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
    printf("Init of SimpleFS is a success\n\n");

    return dh;
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

            int i;
            for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int); i++){
            
                if(dirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(disk, &ffb, dirblock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                    if(strcmp(ffb.fcb.name, filename) == 0){
                        printf("File already exists, change filename please\n");
                        return NULL;
                    }
                }
            }

            nextdir = dirblock ->header.next_block;
        }
    }
    else{
        printf("Nothing to read, directory is empty");
        return NULL;
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

    DirectoryBlock* dirblock;
    int before_pos;
    int pos_in_file = 0;
    int whichupdate = 0;

    if(firstdirblock ->num_entries < (BLOCK_SIZE - sizeof(BlockHeader) -
                                        sizeof(FileControlBlock) -sizeof(int)) 
                                        / sizeof(int)){ //if it's ok i can put new file block in FirstDirectoryBlock
        
        int i;
        for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) - sizeof(FileControlBlock) -sizeof(int)) / sizeof(int); i++){
            if(firstdirblock ->file_blocks[i] == 0){
                firstdirblock ->num_entries++;
                firstdirblock ->file_blocks[i] = free_block_to_mem;
                updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
                goto A;
            }
        }

    }else{
        
        int nextdir = firstdirblock ->header.next_block;
        whichupdate = 1;

        while(1){

            if(nextdir == -1)
                break;

            if(DiskDriver_readBlock(disk, dirblock, nextdir) == -1){        //need to update new information on disk to keep in memory new changes
                printf("impossible to read directory after firstdirectory\n");
                return NULL;
            }

            before_pos = nextdir;
            pos_in_file++;

            int i;
            for(i = 0; i < (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int); i++){
                if(dirblock ->file_blocks[i] == 0){  //if block is empty i write
                    firstdirblock ->num_entries++;
                    updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
                    dirblock ->file_blocks[i] = free_block_to_mem;
                    updateBlockDisk(disk, dirblock, nextdir);
                    goto A;
                }
            }

            nextdir = dirblock ->header.next_block;
        }
    }

    //This is the case where i can't find free spaces before, i should create a new directory e put in position 0 the new block

    DirectoryBlock dirblock_new = { 0 };
    dirblock_new.header.next_block = -1;
    dirblock_new.header.previous_block = before_pos;
    dirblock_new.header.block_in_file = pos_in_file;
    dirblock_new.file_blocks[0] = free_block_to_mem;

    int pos_newdir;
    if((pos_newdir = DiskDriver_getFreeBlock(disk, 0)) == -1){
        printf("Cannot find free space for new directory\n");
        return NULL;
    }

    if(DiskDriver_writeBlock(disk, &dirblock_new, pos_newdir) == -1){
        printf("Cannot write on disk new directory\n");
        return NULL;
    }

    if(whichupdate == 0){
        firstdirblock ->header.next_block = pos_newdir;
        firstdirblock ->num_entries++;
        updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
    }
    else{
        dirblock ->header.next_block = pos_newdir;
        updateBlockDisk(disk, &dirblock_new, pos_newdir);
        firstdirblock ->num_entries++;
        updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
    }


A:  ;
    FileHandle* filehandle = (FileHandle*)malloc(sizeof(FileHandle));
    filehandle ->directory = firstdirblock;
    filehandle ->fcb = &newFile;
    filehandle ->pos_in_file = 0;
    filehandle ->sfs = d ->sfs;

    return filehandle;
    
}

FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename){

    if(d == NULL || filename == NULL){
        printf("Parameters in input are wrong\n");
        return NULL;
    }

    SimpleFS* sfs = d ->sfs;
    FirstDirectoryBlock* firstDirBlock = d ->dcb;
    FileHandle* filehandle_toFind = (FileHandle*)malloc(sizeof(FileHandle));

    if(firstDirBlock ->num_entries > 0){
        
        filehandle_toFind ->sfs = sfs;  //Setting FileHandle for file to find
        filehandle_toFind ->directory = firstDirBlock;
        filehandle_toFind ->pos_in_file = 0;

        FirstFileBlock ffb;
        int i;
        
        for(i = 0; i < firstDirBlock ->num_entries; i++){
            
            if(firstDirBlock ->file_blocks[i] > 0 && (DiskDriver_readBlock(sfs ->disk, &ffb, firstDirBlock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                if(strcmp(ffb.fcb.name, filename) == 0){
                    printf("File found to be opened\n");
                    filehandle_toFind ->fcb = &ffb;
                    return filehandle_toFind;
                }
            }
        }

        int nextdir = firstDirBlock ->header.next_block;
        DirectoryBlock* dirblock;

        while(1){

            if(nextdir == -1)
                break;

            if(DiskDriver_readBlock(sfs ->disk, dirblock, nextdir) == -1){
                printf("impossible to read directories after firstdirectory\n");
                return NULL;
            }

            int i;
            for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int); i++){
            
                if(dirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(sfs ->disk, &ffb, dirblock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                    if(strcmp(ffb.fcb.name, filename) == 0){
                        printf("File found to be opened\n");
                        filehandle_toFind ->fcb = &ffb;
                        return filehandle_toFind;
                    }
                }
            }

            nextdir = dirblock ->header.next_block;
        }
    }
    else{
        printf("Nothing to open, directory is empty\n");
        return NULL;
    }
    
    printf("The file doesn't exist, please try with a different file\n");
    free(filehandle_toFind);
    return NULL;
}

int SimpleFS_readDir(char** names, DirectoryHandle* d){

    FirstDirectoryBlock* fdb = d ->dcb;
    DiskDriver* disk = d ->sfs ->disk;

    FirstFileBlock file_to_read;
    int pos_in_names = 0;

    int i;
    for(i = 0; i < fdb ->num_entries; i++){

        if(fdb ->file_blocks[i] > 0 && DiskDriver_readBlock(disk, &file_to_read, fdb ->file_blocks[i]) == -1){
            names[pos_in_names] = file_to_read.fcb.name;
            pos_in_names++;
        }
    }

    int nextDir = fdb ->header.next_block;
    DirectoryBlock db;
    
    while(nextDir != -1){
        
        if(DiskDriver_readBlock(disk, &db, nextDir) == -1){
            printf("Cannot read directory\n");
            return -1;
        }

        int i;
        for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int); i++){
            if(db.file_blocks[i] > 0 && DiskDriver_readBlock(disk, &file_to_read, db.file_blocks[i]) == -1){
                names[pos_in_names] = file_to_read.fcb.name;
                pos_in_names++;
            }
        }

        nextDir = db.header.next_block;
    }
    
    return 0;
    
}

int SimpleFS_mkDir(DirectoryHandle* d, char* dirname){

    if(d == NULL || dirname == NULL){
        printf("directory handle or dirname are null\n");
        return -1;
    }

    FirstDirectoryBlock fd;    //used from reading directories from disk
    FirstDirectoryBlock* firstdirblock = d ->dcb;
    DiskDriver* disk = d -> sfs ->disk;

    if(firstdirblock ->num_entries > 0){
        int i;
        for(i = 0; i < firstdirblock ->num_entries; i++){
            if(firstdirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(disk, &fd, firstdirblock ->file_blocks[i])) == -1){
                if(strcmp(fd.fcb.name, dirname) == 0){
                        printf("Directory already exists, change dirname please\n");
                        return -1;
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
                return -1;
            }

            int i;
            for(i = 0; i < (BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int)); i++){
            
                if(dirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(disk, &fd, dirblock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                    if(strcmp(fd.fcb.name, dirname) == 0){
                        printf("File already exists, change filename please\n");
                        return -1;
                    }
                }
            }

            nextdir = dirblock ->header.next_block;
        }
    }

    //if arrived here i haven't found any directory with same dirname

    int free_block_to_mem;
    if((free_block_to_mem = DiskDriver_getFreeBlock(disk, 0)) == -1){
        printf("No free blocks in the disk\n");
        return -1;
    }

    FirstDirectoryBlock new_directory = { 1 };
    new_directory.header.block_in_file = 0;
    new_directory.header.next_block = -1;
    new_directory.header.previous_block = -1;
    strcpy(new_directory.fcb.name, dirname);
    new_directory.fcb.directory_block = firstdirblock ->fcb.block_in_disk;
    new_directory.fcb.block_in_disk = free_block_to_mem;

    if(DiskDriver_writeBlock(disk, &new_directory, free_block_to_mem) == -1){
        printf("Impossible to write new directory on disk\n");
        return -1;
    }

    int whichupdate = 0;
    DirectoryBlock* dirblock;

    if(firstdirblock ->num_entries < (BLOCK_SIZE - sizeof(BlockHeader) -
                                        sizeof(FileControlBlock) -sizeof(int)) 
                                        / sizeof(int)){ //if it's ok i can put new directory block in FirstDirectoryBlock
        
        int i;
        for(i = 0; i < firstdirblock ->num_entries; i++){
            if(firstdirblock ->file_blocks[i] == 0){
                firstdirblock ->num_entries++;
                firstdirblock ->file_blocks[i] = free_block_to_mem;
                updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
                break;
            }
        }

    }else{
        
        int nextdir = firstdirblock ->header.next_block;
        whichupdate = 1;

        while(1){

            if(nextdir == -1)
                break;

            if(DiskDriver_readBlock(disk, dirblock, nextdir) == -1){
                printf("impossible to read directory after firstdirectory\n");
                return -1;
            }

            int i;
            for(i = 0; i < (BLOCK_SIZE-sizeof(BlockHeader))/sizeof(int); i++){
                if(dirblock ->file_blocks[i] == 0){  //if block is empty i write
                    firstdirblock ->num_entries++;
                    updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
                    dirblock ->file_blocks[i] = free_block_to_mem;
                    updateBlockDisk(disk, dirblock, nextdir);
                    break;
                }
            }

            nextdir = dirblock ->header.next_block;
        }
    }

    return 0;
}

int SimpleFS_close(FileHandle* f){      //Use to free FileHandle
    free(f);
    return 0;
}