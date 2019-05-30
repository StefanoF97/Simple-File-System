#include "simplefs.h"
#include "disk_driver.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  
    printf("FirstBlock size %ld\n", sizeof(FirstFileBlock));
    printf("DataBlock size %ld\n", sizeof(FileBlock));
    printf("FirstDirectoryBlock size %ld\n", sizeof(FirstDirectoryBlock));
    printf("DirectoryBlock size %ld\n\n", sizeof(DirectoryBlock));

    printf("Starting test for SimpleFS....\n\n");

    printf("TESTS FOR SimpleFS_init AND SimpleFS_format\n");
    
    DiskDriver disk;
    SimpleFS sfs;

    DiskDriver_init(&disk, "disk.txt", 1024);
    printf("Disk has been created(SUCCESS)\n");

    DirectoryHandle* dir_root = SimpleFS_init(&sfs, &disk);
    if(dir_root == NULL){
        printf("Perhaps i need to format(i'll write something to read)...\n");
        SimpleFS_format(&sfs);
        dir_root = SimpleFS_init(&sfs, &disk);
        if(dir_root == NULL){
            printf("Problems in init disk(ERROR)\n\n");
            return -1;
        }
    }

    printf("TESTS FOR SimpleFS_createFile\n\n");
    printf("Creo un file chiamato first_file\n");
    FileHandle* filehandle1 = SimpleFS_createFile(dir_root, "first_file");
    if(filehandle1 == NULL){
        printf("Error in creating a file(ERROR)\n");
        exit(1);
    }
    else{
        printf("FileHandle creato correttamente(SUCCESS)\n");
        printf("Posizione della directory nel file: %d\n\n", filehandle1 ->directory ->header.block_in_file);
        SimpleFS_close(filehandle1);
    }
    
    printf("Creo un altro file chiamato first_file\n");
    FileHandle* filehandle2 = SimpleFS_createFile(dir_root, "first_file");
    if(filehandle2 != NULL){
        printf("FileHandle creato correttamente(ERROR)\n\n");
        SimpleFS_close(filehandle2);
    }
    else{
        printf("FileHandle non creato perchè il file è già esistente(SUCCESS)\n\n");
    }
    
    
    /*
    int res = SimpleFS_mkDir(dir_root, "subDir1");
    if(res == -1){
        printf("Error in creating new directory");
    }
    */
    
    return 0;

}
