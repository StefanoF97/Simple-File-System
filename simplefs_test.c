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
    
    printf("Creo un primo file chiamato first_file\n");
    FileHandle* filehandle = SimpleFS_createFile(dir_root, "first_file");
    if(filehandle == NULL){
        printf("Error in creating a file(ERROR)\n");
    }
    else{
        printf("FileHandle creato correttamente(SUCCESS)\n");
        printf("Nome del file creato: %s\n\n", filehandle ->fcb ->fcb.name);
        SimpleFS_close(filehandle);
    }
    
    printf("Creo un secondo file chiamato first_file\n");
    filehandle = SimpleFS_createFile(dir_root, "first_file");
    if(filehandle != NULL){
        printf("FileHandle creato correttamente(ERROR)\n\n");
        SimpleFS_close(filehandle);
    }
    else{
        printf("FileHandle non creato perchè il file è già esistente(SUCCESS)\n\n");
    }

    printf("Creo un terzo file chiamato second_file\n");
    filehandle = SimpleFS_createFile(dir_root, "second_file");
    if(filehandle != NULL){
        printf("FileHandle creato correttamente(SUCCESS)\n\n");
        printf("Nome del file creato: %s\n\n", filehandle ->fcb ->fcb.name);
        SimpleFS_close(filehandle);
    }
    else{
        printf("FileHandle non creato perchè il file è già esistente(ERROR)\n\n");
    }

    printf("\n\n");

    printf("TESTS FOR SimpleFS_openFile\n\n");

    printf("Provo ad aprie il file 'first_file' nella directory root\n");
    FileHandle* file_to_open = SimpleFS_openFile(dir_root, "first_file");
    if(file_to_open == NULL){
        printf("Error in opening file 'first_file'(ERROR)\n");
    }
    else{
        printf("File 'first_file' aperto (SUCCESS)\n");
        printf("Nome del file aperto -> %s\n\n", file_to_open ->fcb ->fcb.name);
        SimpleFS_close(file_to_open);
    }

    printf("Provo ad aprie il file 'second_file' nella directory root\n");
    file_to_open = SimpleFS_openFile(dir_root, "second_file");
    if(file_to_open == NULL){
        printf("Error in opening file 'second_file'(ERROR)\n");
    }
    else{
        printf("File 'second_file' aperto (SUCCESS)\n");
        printf("Nome del file aperto -> %s\n\n", file_to_open ->fcb ->fcb.name);
        SimpleFS_close(file_to_open);
    }

    printf("Provo ad aprie il file 'fourth_file' nella directory root\n");
    file_to_open = SimpleFS_openFile(dir_root, "fourth_file");
    if(file_to_open == NULL){
        printf("Error in opening file 'fourth_file'(SUCCESS)\n\n");
    }
    else{
        printf("File 'fourth_file' aperto (ERROR)\n");
        printf("Nome del file aperto -> %s\n\n\n", file_to_open ->fcb ->fcb.name);
        SimpleFS_close(file_to_open);
    }
    
    /*
    int res = SimpleFS_mkDir(dir_root, "subDir1");
    if(res == -1){
        printf("Error in creating new directory");
    }
    */
    
    return 0;

}
