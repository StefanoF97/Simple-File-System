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

    printf("Tests for SimpleFS_init and SimpleFS_format\n");
    
    DiskDriver disk;
    SimpleFS sfs;

    DiskDriver_init(&disk, "disk.txt", 512);
    printf("Disk has been created\n");

    DirectoryHandle* dir_root = SimpleFS_init(&sfs, &disk);
    if(dir_root == NULL){
        printf("Perhaps i need to format(i'll write something to read)...\n");
        SimpleFS_format(&sfs);
        dir_root = SimpleFS_init(&sfs, &disk);
        if(dir_root == NULL){
            printf("Problems in init disk\n\n");
            return -1;
        }
    }

    printf("Tests for SimpleFS_createFile\n");

    FileHandle* filehandle1 = SimpleFS_createFile(dir_root, "first_file");
    if(filehandle1 == NULL){
        printf("Error in creating a file\n");
    }

    int res = SimpleFS_mkDir(dir_root, "subDir1");
    if(res == -1){
        printf("Error in creating new directory");
    }


    return 0;

}
