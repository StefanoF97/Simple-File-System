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
    printf("\n");

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
    printf("\n");

    printf("TESTS FOR SimpleFS_createFile\n\n");
    
    printf("Creo un primo file chiamato first_file\n");
    FileHandle* filehandle = SimpleFS_createFile(dir_root, "first_file\0");
    if(filehandle == NULL){
        printf("Error in creating a file(ERROR)\n");
    }
    else{
        printf("FileHandle creato correttamente(SUCCESS)\n");
        printf("Nome del file creato: %s\n\n", filehandle ->fcb ->fcb.name);
        SimpleFS_close(filehandle);
    }
    
    printf("Creo un secondo file chiamato first_file\n");
    filehandle = SimpleFS_createFile(dir_root, "first_file\0");
    if(filehandle != NULL){
        printf("FileHandle creato correttamente(ERROR)\n\n");
        SimpleFS_close(filehandle);
    }
    else{
        printf("FileHandle non creato perchè il file è già esistente(SUCCESS)\n\n");
    }

    printf("Creo un terzo file chiamato second_file\n");
    filehandle = SimpleFS_createFile(dir_root, "second_file\0");
    if(filehandle != NULL){
        printf("FileHandle creato correttamente(SUCCESS)\n\n");
        printf("Nome del file creato: %s\n\n", filehandle ->fcb ->fcb.name);
        SimpleFS_close(filehandle);
    }
    else{
        printf("FileHandle non creato perchè il file è già esistente(ERROR)\n\n");
    }

    printf("Creo un quarto file chiamato third_file\n");
    filehandle = SimpleFS_createFile(dir_root, "third_file\0");
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
    FileHandle* file_to_open = SimpleFS_openFile(dir_root, "first_file\0");
    if(file_to_open == NULL){
        printf("Error in opening file 'first_file'(ERROR)\n");
    }
    else{
        printf("File 'first_file' aperto (SUCCESS)\n");
        printf("Nome del file aperto -> %s\n\n", file_to_open ->fcb ->fcb.name);
        SimpleFS_close(file_to_open);
    }

    printf("Provo ad aprie il file 'second_file' nella directory root\n");
    file_to_open = SimpleFS_openFile(dir_root, "second_file\0");
    if(file_to_open == NULL){
        printf("Error in opening file 'second_file'(ERROR)\n");
    }
    else{
        printf("File 'second_file' aperto (SUCCESS)\n");
        printf("Nome del file aperto -> %s\n\n", file_to_open ->fcb ->fcb.name);
        SimpleFS_close(file_to_open);
    }

    printf("Provo ad aprie il file 'fourth_file' nella directory root\n");
    file_to_open = SimpleFS_openFile(dir_root, "fourth_file\0");
    if(file_to_open == NULL){
        printf("Error in opening file 'fourth_file'(SUCCESS)\n\n");
    }
    else{
        printf("File 'fourth_file' aperto (ERROR)\n");
        printf("Nome del file aperto -> %s\n\n\n", file_to_open ->fcb ->fcb.name);
        SimpleFS_close(file_to_open);
    }

    printf("Provo ad aprie il file 'third_file' nella directory root\n");
    file_to_open = SimpleFS_openFile(dir_root, "third_file\0");
    if(file_to_open == NULL){
        printf("Error in opening file 'third_file'(ERROR)\n\n");
    }
    else{
        printf("File 'third_file' aperto (SUCCESS)\n");
        printf("Nome del file aperto -> %s\n\n\n", file_to_open ->fcb ->fcb.name);
        SimpleFS_close(file_to_open);
    }
    
    printf("\n\n");

    printf("TESTS FOR SimpleFS_readDir\n");

    char** names =(char**)malloc(sizeof(char*) * dir_root ->dcb ->num_entries);
    int ret, i;

    ret = SimpleFS_readDir(names, dir_root);
    if(ret == -1){
        printf("Errore nella lettura della directory (ERROR)\n");
    }
    else{
        printf("\nLettura della directory avvenuta\n");
        printf("File trovati: ");
        for(i = 0; i < dir_root ->dcb ->num_entries; i++){
            printf("%s - ", names[i]);
        }
    }
    for(i = 0; i < dir_root ->dcb ->num_entries; i++){
       free(names[i]);
    }
    free(names);

    printf("\n\n");

    
    printf("TESTS FOR SimpleFS_mkDir\n\n");
    
    printf("Creo una sottocartella per la directory 'dir_root' chiamata 'subDir1'\n");
    int res = SimpleFS_mkDir(dir_root, "subDir1");
    if(res == -1){
        printf("Errore nella creazione della new_directory\n");
        res = SimpleFS_mkDir(dir_root, "subDir1");
        if(res == -1){
            printf("it's impossible to create new directory\n");
        }
    }
    printf("\n\n");

    printf("TESTS FOR SimpleFS_changeDir\n\n");
    printf("Provo ad andare dalla directory root alla directory subDir2(non esiste)\n");

    res = SimpleFS_changeDir(dir_root, "subDir2");
    if(res == -1){
        printf("sub-directory non trovata(SUCCESS)\n\n");
    }
    else{
        printf("Sub-directory trovata(ERROR)\n");
        printf("Directory corrente: %s\n", dir_root ->dcb ->fcb.name);
    }

    printf("Provo ad andare dalla directory root alla directory subDir1(esiste)\n");
    res = SimpleFS_changeDir(dir_root, "subDir1");
    if(res == -1){
        printf("sub-directory non trovata(ERROR)\n");
    }
    else{
        printf("Sub-directory trovata(SUCCESS)\n");
        printf("Directory corrente: %s\n\n", dir_root ->dcb ->fcb.name);
    }

    printf("Provo ad andare alla directory padre di subDir1 (figlia della radice)\n");
    res = SimpleFS_changeDir(dir_root, "..");
    if(res == -1){
        printf("directory padre non trovata(ERROR)\n");
    }
    else{
        printf("Parent-directory trovata(SUCCESS)\n");
        printf("Directory corrente: %s\n\n", dir_root ->dcb ->fcb.name);
    }

    printf("Provo ad andare alla directory padre delle radice (non esiste)\n");
    res = SimpleFS_changeDir(dir_root, "..");
    if(res == -1){
        printf("directory padre non trovata(SUCCESS)\n\n");
    }
    else{
        printf("Parent-directory trovata(ERROR)\n");
        printf("Directory corrente: %s\n\n", dir_root ->dcb ->fcb.name);
    }

    free(dir_root ->dcb);
    free(dir_root);
    
    return 0;
}
