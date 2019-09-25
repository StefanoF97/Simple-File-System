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
        //free(dh);           //IMPORTANTE E DA VERIFICARE
        //free(fsb);          //IMPORTANTE E DA VERIFICARE
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
    //bzero(fs ->disk, fs ->disk ->header ->bitmap_entries);

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

    if(firstdirblock ->num_entries >= 0){
        
        //taking FirstDirectoryBlock (and after directory block using blockheader) i can try to find if there is just the same file i want to create
        int i;
        
        for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) -sizeof(FileControlBlock) -sizeof(int))/sizeof(int); i++){
            
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

    FirstFileBlock* newFile = calloc(1, sizeof(FirstFileBlock));
    newFile ->header.block_in_file = 0;
    newFile ->header.next_block = -1;
    newFile ->header.previous_block = -1;
    newFile ->fcb.is_dir = 0;
    strcpy(newFile ->fcb.name, filename);
    newFile ->fcb.block_in_disk = free_block_to_mem;
    newFile ->fcb.directory_block = firstdirblock ->fcb.block_in_disk;    //because starting from FirstDirectoryBlock

    if(DiskDriver_writeBlock(disk, newFile, free_block_to_mem) == -1){
        printf("Cannot write new file on disk\n");
        return NULL;
    }

    DirectoryBlock* dirblock;
    int before_pos = firstdirblock ->fcb.block_in_disk;
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
            
            whichupdate = 1;
            nextdir = dirblock ->header.next_block;
        }
    }

    //This is the case where i can't find free spaces before, i should create a new directory e put in position 0 the new block
    printf("caso in cui non c'è spazio\n");

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
        updateBlockDisk(disk, dirblock, before_pos);
        dirblock_new.file_blocks[0] = free_block_to_mem;
        updateBlockDisk(disk, &dirblock_new, pos_newdir);
        firstdirblock ->num_entries++;
        updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
    }

A:  ;
    FileHandle* filehandle = (FileHandle*)malloc(sizeof(FileHandle));
    filehandle ->directory = firstdirblock;
    filehandle ->fcb = newFile;
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
    FirstFileBlock* ffb;

    if(firstDirBlock ->num_entries > 0){
        
        filehandle_toFind ->sfs = sfs;  //Setting FileHandle for file to find
        filehandle_toFind ->directory = firstDirBlock;
        filehandle_toFind ->pos_in_file = 0;

        ffb = (FirstFileBlock*)malloc(sizeof(FirstFileBlock));
        int i;
        
        for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) -sizeof(FileControlBlock) -sizeof(int))/sizeof(int); i++){
            
            if(firstDirBlock ->file_blocks[i] > 0 && (DiskDriver_readBlock(sfs ->disk, ffb, firstDirBlock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                if(strcmp(ffb ->fcb.name, filename) == 0){
                    printf("File found to be opened\n");
                    filehandle_toFind ->fcb = ffb;
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
            
                if(dirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(sfs ->disk, ffb, dirblock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                    if(strcmp(ffb ->fcb.name, filename) == 0){
                        printf("File found to be opened\n");
                        filehandle_toFind ->fcb = ffb;
                        return filehandle_toFind;
                    }
                }
            }

            nextdir = dirblock ->header.next_block;
        }
    }
    else{
        printf("Nothing to open, directory is empty\n");
        //free(filehandle_toFind);   //IMPORTANTE E DA VERIFICARE
        return NULL;
    }
    
    printf("The file doesn't exist, please try with a different file\n");
    free(filehandle_toFind);
    //free(ffb);                    //IMPORTANTE E DA VERIFICARE
    return NULL;
}

int SimpleFS_seek(FileHandle* f, int pos){

    if(f == NULL){
        printf("FileHandle is NULL, nothing to seek\n");
        return -1;
    }

    if(pos > f ->fcb ->fcb.size_in_bytes){
        printf("the position is greater than the written bytes\n");
        return -1;
    }

    f ->pos_in_file = pos;

    return 0;
}


int SimpleFS_changeDir(DirectoryHandle* d, char* dirname){

    if(d == NULL || dirname == NULL){
        printf("DirectoryHandle is NULL or dirname is NULL\n");
        return -1;
    }

    if(strcmp(dirname, "..") == 0){
        
        if(d ->directory == NULL){  //if it's NULL, there isn't a parent directory
            printf("This is root directory so it's impossible to read parent directory\n");
            return -1;
        }

        d ->pos_in_block = 0;       //SIDE-EFFECT
        d ->dcb = d ->directory;    //SO THE DIRECTORY BECOMES THE FATHER

        FirstDirectoryBlock ParentDir;
        SimpleFS* simplefs = d ->sfs;

        if(d ->dcb ->fcb.directory_block != -1){

            if(DiskDriver_readBlock(simplefs ->disk, &ParentDir, d ->dcb ->fcb.directory_block) != -1){
                
                printf("Impossible to read parent directory from disk\n");
                d ->directory = NULL;
                return -1;
            
            }
            else{
                
                d ->directory = &ParentDir;
                return 0;
            
            }
        }
        else{
            
            d ->directory = NULL;
            return 0;
        }
    }

    if(d ->dcb ->num_entries <= 0){
        printf("The directory is empty, it's impossible to change directory\n");
        return -1;
    }

    SimpleFS* sfs = d ->sfs;
    FirstDirectoryBlock* firstDirBlock = d ->dcb;
    FirstDirectoryBlock* DirToFind = (FirstDirectoryBlock*)malloc(sizeof(FirstDirectoryBlock)); 

    if(firstDirBlock ->num_entries > 0){

        int i;
        for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) -sizeof(FileControlBlock) -sizeof(int))/sizeof(int); i++){
            if(firstDirBlock ->file_blocks[i] > 0 && (DiskDriver_readBlock(sfs ->disk, DirToFind, firstDirBlock ->file_blocks[i])) != -1){
                if(strcmp(DirToFind ->fcb.name, dirname) == 0){
                    printf("Directory found, making side-effect\n");
                    d ->pos_in_block = 0;
                    d ->directory = firstDirBlock;
                    d ->dcb = DirToFind;
                    return 0;
                }
            }
        }

        int nextdir = firstDirBlock ->header.next_block;
        DirectoryBlock dirblock;

        while(1){

            if(nextdir == -1)
                break;

            if(DiskDriver_readBlock(sfs ->disk, &dirblock, nextdir) == -1){
                printf("impossible to read directories after firstdirectory\n");
                return -1;
            }

            int i;
            for(i = 0; i < (BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int)); i++){
            
                if(dirblock.file_blocks[i] > 0 && (DiskDriver_readBlock(sfs ->disk, DirToFind, dirblock.file_blocks[i]) != -1)){  //if block is empty is useless to read it
                    if(strcmp(DirToFind ->fcb.name, dirname) == 0){
                        printf("Directory found, making side-effect\n");
                        d ->pos_in_block = 0;
                        d ->directory = firstDirBlock;
                        d ->dcb = DirToFind;
                        return 0;
                    }
                }
            }

            nextdir = dirblock.header.next_block;
        }

    }

    printf("Impossible to change directory, there's not in the disk\n");
    return -1;
}


int SimpleFS_readDir(char** names, DirectoryHandle* d){

    FirstDirectoryBlock* fdb = d ->dcb;
    DiskDriver* disk = d ->sfs ->disk;
    
    FirstFileBlock file_to_read;
    int pos_in_names = 0;

    if(fdb ->num_entries > 0){
        int i;
        for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) -sizeof(FileControlBlock) -sizeof(int))/sizeof(int); i++){

            if(fdb ->file_blocks[i] > 0 && DiskDriver_readBlock(disk, &file_to_read, fdb ->file_blocks[i]) != -1){
                names[pos_in_names] = strdup(file_to_read.fcb.name);
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
            for(i = 0; i < BLOCK_SIZE - sizeof(BlockHeader)/sizeof(int); i++){
                if(db.file_blocks[i] > 0 && DiskDriver_readBlock(disk, &file_to_read, db.file_blocks[i]) != -1){
                    names[pos_in_names] = strdup(file_to_read.fcb.name);
                    pos_in_names++;
                }
            }

            nextDir = db.header.next_block;
        }
    }
    else{
        printf("La directory è vuota\n");
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
        for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) -sizeof(FileControlBlock) -sizeof(int))/sizeof(int); i++){
            if(firstdirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(disk, &fd, firstdirblock ->file_blocks[i])) != -1){
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
    //printf("passato qui(caso in cui non si trovano directory con lo stesso nome)\n");

    int free_block_to_mem;
    if((free_block_to_mem = DiskDriver_getFreeBlock(disk, disk ->header ->first_free_block)) == -1){
        printf("No free blocks in the disk\n");
        return -1;
    }

    FirstDirectoryBlock* new_directory = calloc(1, sizeof(FirstDirectoryBlock));
    new_directory ->fcb.is_dir = 1;
    new_directory ->header.block_in_file = 0;
    new_directory ->header.next_block = -1;
    new_directory ->header.previous_block = -1;
    strcpy(new_directory ->fcb.name, dirname);
    new_directory ->fcb.directory_block = firstdirblock ->fcb.block_in_disk;
    new_directory ->fcb.block_in_disk = free_block_to_mem;
    
    if(DiskDriver_writeBlock(disk, new_directory, free_block_to_mem) == -1){
        printf("Impossible to write new directory on disk\n");
        return -1;
    }

    //free(new_directory);    //IMPORTANTE E DA VERIFICARE

    int whichupdate = 0;
    int before_pos = firstdirblock ->fcb.block_in_disk;
    int pos_in_file = 0;
    DirectoryBlock* dirblock;

    if(firstdirblock ->num_entries < (BLOCK_SIZE - sizeof(BlockHeader) -
                                        sizeof(FileControlBlock) -sizeof(int)) 
                                        / sizeof(int)){ //if it's ok i can put new directory block in FirstDirectoryBlock
        
        //printf("passato qui(caso in cui aggiungo la directory alla first directory)\n");
        int i;
        for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) - sizeof(FileControlBlock) -sizeof(int)) / sizeof(int); i++){
            if(firstdirblock ->file_blocks[i] == 0){
                firstdirblock ->num_entries++;
                firstdirblock ->file_blocks[i] = free_block_to_mem;
                updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
                return 0;
            }
        }

    }else{
        
        printf("passato qui(caso in cui aggiungo le directory alle sub-directory)\n");
        int nextdir = firstdirblock ->header.next_block;
        
        while(1){

            if(nextdir == -1)
                break;

            if(DiskDriver_readBlock(disk, dirblock, nextdir) == -1){
                printf("impossible to read directory after firstdirectory\n");
                return -1;
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
                    return 0;
                }
            }

            whichupdate = 1;
            nextdir = dirblock ->header.next_block;
        }
    }

    //This is the case like in CreateFile where i can't find free space
    printf("passato qui(caso di mancato spazio\n");

    DirectoryBlock dirblock_new = { 0 };
    dirblock_new.header.next_block = -1;
    dirblock_new.header.previous_block = before_pos;
    dirblock_new.header.block_in_file = pos_in_file;
    dirblock_new.file_blocks[0] = free_block_to_mem;

    int pos_newdir;
    if((pos_newdir = DiskDriver_getFreeBlock(disk, disk ->header ->first_free_block)) == -1){
        printf("Cannot find free space for new directory\n");
        return -1;
    }

    if(DiskDriver_writeBlock(disk, &dirblock_new, pos_newdir) == -1){
        printf("Cannot write on disk new directory\n");
        return -1;
    }

    if(whichupdate == 0){
        firstdirblock ->header.next_block = pos_newdir;
        firstdirblock ->num_entries++;
        updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
    }
    else{
        dirblock ->header.next_block = pos_newdir;
        updateBlockDisk(disk, dirblock, before_pos);
        dirblock_new.file_blocks[0] = free_block_to_mem;
        updateBlockDisk(disk, &dirblock_new, pos_newdir);
        firstdirblock ->num_entries++;
        updateBlockDisk(disk, firstdirblock, firstdirblock ->fcb.block_in_disk);
    }
    
    return 0;
}

int SimpleFS_close(FileHandle* f){      //Use to free FileHandle
    free(f->fcb);
    free(f);
    return 0;
}

int SimpleFS_write(FileHandle* f, void* data, int size){

    if(f == NULL || data == NULL || size <= 0){
        printf("Parametri passati a SimpleFS_write errati\n");
        return -1;
    }

    int bytes_written = 0;
    int bytes_to_write = size;
    int difference = f ->pos_in_file;

    if(difference < BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader) && bytes_to_write <= BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader) - difference){     //i tre casi possibile per il first file block
        memcpy(f ->fcb ->data + difference, data, bytes_to_write);
        bytes_written = bytes_to_write;
        f ->pos_in_file += bytes_written;
        f ->fcb ->fcb.size_in_bytes = f ->pos_in_file;
        updateBlockDisk(f ->sfs ->disk, f ->fcb, f ->fcb ->fcb.block_in_disk);
        return bytes_written;
    }
    else if(difference < BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader) && bytes_to_write > BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader) - difference){
        memcpy(f ->fcb ->data + difference, data, BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader) - difference);
        bytes_written += (BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader) - difference);
        bytes_to_write = size - bytes_written;
        updateBlockDisk(f ->sfs ->disk, f ->fcb, f ->fcb ->fcb.block_in_disk);
        difference = 0;
    }
    else{
        difference -= BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader);
    }

    //this is the case when i write to the other file block 
    
    int next = f ->fcb ->header.next_block;
    int blockIndisk = f -> fcb ->fcb.block_in_disk;
    int blockInfile = f ->fcb ->header.block_in_file;
    int is_this_first_block = 0;
    FileBlock fileblock_to_mem;
   
    if(next == -1){
        is_this_first_block = 1;
    }

    while(bytes_written < size){

        //printf("Sono entrato qui per ' file_write2 ' (SimpleFS_write) \n");
        
        if(next == -1){     //this is the case when i haven't space enough
            
            FileBlock f1 = { 0 };   //new FileBlock necessary
            f1.header.block_in_file = blockInfile + 1;
            f1.header.next_block = -1;
            f1.header.previous_block = blockIndisk;
            
            next = DiskDriver_getFreeBlock(f ->sfs ->disk, blockIndisk);

            if(is_this_first_block == 1){   //it means i'm in FirstFileBlock
                
                f ->fcb ->header.next_block = next;
                updateBlockDisk(f ->sfs ->disk, f ->fcb, f ->fcb ->fcb.block_in_disk);
                is_this_first_block = 0;  //From this point i'll never be in FirstFileBLock
            
            }
            else{
                
                fileblock_to_mem.header.next_block = next;              //update fileblock_to_mem to point to new file block
                updateBlockDisk(f ->sfs ->disk, &fileblock_to_mem, blockIndisk);

            }

            DiskDriver_writeBlock(f ->sfs ->disk, &f1, next);
            fileblock_to_mem = f1;
        }

        //Write conditions are missing, it's missing the update of pos_in_file to not cancel new data

        if(difference < BLOCK_SIZE - sizeof(BlockHeader) && bytes_to_write <= BLOCK_SIZE - sizeof(BlockHeader) - difference){     //i tre casi possibile per il first file block
            //printf("Sono passato qui per file_write2\n");
            memcpy(fileblock_to_mem.data + difference, data + bytes_written, bytes_to_write);
            bytes_written += bytes_to_write;
            f ->pos_in_file += bytes_written;
            f ->fcb ->fcb.size_in_bytes = f ->pos_in_file;
            updateBlockDisk(f ->sfs ->disk, f ->fcb, f ->fcb ->fcb.block_in_disk);
            updateBlockDisk(f ->sfs ->disk, &fileblock_to_mem, next);
            return bytes_written;
        }
        else if(difference < BLOCK_SIZE - sizeof(BlockHeader) && bytes_to_write > BLOCK_SIZE - sizeof(BlockHeader) - difference){
            memcpy(fileblock_to_mem.data + difference, data + bytes_written, BLOCK_SIZE - sizeof(BlockHeader) - difference);
            bytes_written += (BLOCK_SIZE - sizeof(BlockHeader) - difference);
            bytes_to_write = size - bytes_written;
            updateBlockDisk(f ->sfs ->disk, &fileblock_to_mem, next);
            difference = 0;
        }
        else{
            difference -= BLOCK_SIZE - sizeof(BlockHeader);
        }

        printf("next: %d\n", next);
        blockIndisk = next;
        next = fileblock_to_mem.header.next_block;
        blockInfile = fileblock_to_mem.header.block_in_file;
    }

    return bytes_written;
}

int SimpleFS_read(FileHandle* f, void* data, int size){

    if(f == NULL || data == NULL || size < 0){
        printf("Parameters are wrong for SimpleFS_read\n");
        return -1;
    }

    FirstFileBlock* ffb = f ->fcb;
    int read_bytes;
    
    if(size < BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader)){

        memcpy((char*)data, ffb ->data, size);
        read_bytes = size;
        size = -1;
    
    }
    else{
        
        memcpy((char*)data, ffb ->data, BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader)); 
        read_bytes = BLOCK_SIZE - sizeof(FileControlBlock) - sizeof(BlockHeader);
        size -= read_bytes;

    }
    
    int ret, next = ffb ->header.next_block;
    FileBlock fb;

    while(size > 0 && next != -1){

        //printf("Sono entrato qui per ' file_write2 ' (SimpleFS_read) \n");

        ret = DiskDriver_readBlock(f ->sfs ->disk, &fb, next);
        if(ret == -1){
            
            printf("Error in reading block from disk\n");
            return -1;
        
        }

        if(size < BLOCK_SIZE - sizeof(BlockHeader)){

            memcpy((char*)data + read_bytes, fb.data, size);
            read_bytes += size;
            size = -1;

        }
        else{
            
            memcpy((char*)data + read_bytes, fb.data, BLOCK_SIZE-sizeof(BlockHeader));
            read_bytes += BLOCK_SIZE - sizeof(BlockHeader);
            size -= read_bytes;
            
        }

        next = fb.header.next_block;

    }
    
    return read_bytes;

}

int SimpleFS_remove(DirectoryHandle* d, char* filename){

    //if filename is the name of the root i'll remove all file inside(setting free the bitmap)
    //else i found file/dir with filename and after i remove his files(setting free the bitmap)t

    if(d == NULL || filename == NULL){
        printf("Parameters wrong for SimpleFS_remove\n");
        return -1;
    }

    FirstFileBlock ffb;         //using for reading from disk...
    FirstDirectoryBlock* firstdirblock = d ->dcb;
    DiskDriver* disk = d ->sfs ->disk;
    int found = 0;

    if(firstdirblock ->num_entries >= 0){
        
        //taking FirstDirectoryBlock (and after directory block using blockheader) i can try to find if there is just the same file i want to delete
        int i;
        
        for(i = 0; i < firstdirblock ->num_entries; i++){
            
            if(firstdirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(disk, &ffb, firstdirblock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                if(strcmp(ffb.fcb.name, filename) == 0){
                    printf("File to remove exists\n");
                    found = 1;
                    break;
                }
            }
        }

        if(!found){

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
                for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int); i++){
                
                    if(dirblock ->file_blocks[i] > 0 && (DiskDriver_readBlock(disk, &ffb, dirblock ->file_blocks[i]) != -1)){  //if block is empty is useless to read it
                        if(strcmp(ffb.fcb.name, filename) == 0){
                            printf("File to remove exists\n");
                            found = 1;
                            break;
                        }
                    }
                }

                nextdir = dirblock ->header.next_block;
            }
        }
    }
    else{
        printf("Nothing to read, directory is empty\n");
        return -1;
    }
        
    if(!found){
        printf("The file you want to remove doesn't exist\n");
        return -1;
    }

    //if arrived i've found the file that i want to delete

    if(ffb.fcb.is_dir == 1){        //case directory to delete
        printf("Si tratta di una directory da eliminare\n");
        return RemoveDir((FirstDirectoryBlock*)(&ffb), firstdirblock, disk);
    }
    else{                           //case file to delete
        printf("Si tratta di un file da eliminare\n");
        return RemoveFile(&ffb, firstdirblock, disk);
    }
    
    return -1;

}

int RemoveFile(FirstFileBlock* ffb, FirstDirectoryBlock* fdb, DiskDriver* disk){
    
    if(ffb == NULL || fdb == NULL || disk == NULL){
        printf("Parameters are wrong in RemoveFile\n");
        return -1;
    }

    // 1)eliminare lista file blocks
    
    int ret; 
    int next = ffb ->header.next_block;
    int previous = -1;
    int first_file_block = ffb ->fcb.block_in_disk;
    
    FileBlock fb;
    DiskDriver_freeBlock(disk, ffb ->fcb.block_in_disk);

    while(next != -1){

        ret = DiskDriver_readBlock(disk, &fb, next);
        if(ret == -1){
            printf("Error in reading block from disk\n");
            return -1;
        }

        previous = next;
        next = ffb ->header.next_block;
        DiskDriver_freeBlock(disk, previous);
    }

    // 2)eliminare la entry da file blocks

    DirectoryBlock db;
    next = fdb ->header.next_block;
    int found = 0;

    int i;
    for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) -sizeof(FileControlBlock) -sizeof(int))/sizeof(int); i++){
        if(fdb ->file_blocks[i] == first_file_block){
            fdb ->file_blocks[i] = 0;
            found = 1;
            break;
        }
    }
    
    if(!found){

        while(next != -1){
            
            if(DiskDriver_readBlock(disk, &db, next) == -1){
                printf("Cannot read directory\n");
                return -1;
            }

            for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int); i++){
                if(db.file_blocks[i] == first_file_block){
                    db.file_blocks[i] = 0;
                    break;
                }
            }

            next = db.header.next_block;
        }
    }

    // 3)entries --
    fdb ->num_entries --;
    updateBlockDisk(disk, fdb, fdb ->fcb.block_in_disk);

    return 0;
}

int RemoveDir(FirstDirectoryBlock* ffb, FirstDirectoryBlock* fdb, DiskDriver* disk){
    
    if(ffb == NULL || fdb == NULL || disk == NULL){
        printf("Parameters are wrong in RemoveFile\n");
        return -1;
    }

    //1)search if ffb is empty
    if(ffb ->num_entries != 0){
        printf("The directory is not empty, cannot remove it\n");
        return 0;
    }

    //2)eliminare directory
    int first_file_block = ffb ->fcb.block_in_disk;
    
    DiskDriver_freeBlock(disk, ffb ->fcb.block_in_disk);

    // 3)eliminare la entry da file blocks

    int next = fdb ->header.next_block;
    
    DirectoryBlock db;
    int found = 0;

    int i;
    for(i = 0; i < (BLOCK_SIZE - sizeof(BlockHeader) -sizeof(FileControlBlock) -sizeof(int))/sizeof(int); i++){
        if(fdb ->file_blocks[i] == first_file_block){
            fdb ->file_blocks[i] = 0;
            found = 1;
            break;
        }
    }

    if(!found){ 
        
        while(next != -1){
            
            if(DiskDriver_readBlock(disk, &db, next) == -1){
                printf("Cannot read directory\n");
                return -1;
            }

            for(i = 0; i < BLOCK_SIZE-sizeof(BlockHeader)/sizeof(int); i++){
                if(fdb ->file_blocks[i] == first_file_block){
                    fdb ->file_blocks[i] = 0;
                    break;
                }
            }

            next = db.header.next_block;
        }
    }

    // 4)entries --
    fdb ->num_entries --;

    updateBlockDisk(disk, fdb, fdb ->fcb.block_in_disk);
    return 0;    
    
}