#include "simplefs.h"
#include "disk_driver.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    
    printf("FirstBlock size %ld\n", sizeof(FirstFileBlock));
    printf("DataBlock size %ld\n", sizeof(FileBlock));
    printf("FirstDirectoryBlock size %ld\n", sizeof(FirstDirectoryBlock));
    printf("DirectoryBlock size %ld\n\n", sizeof(DirectoryBlock));
    printf("\n");

    printf("Starting test for SimpleFS....\n\n");

    printf("\e[1;32mTESTS FOR SimpleFS_init AND SimpleFS_format\n\n\e[00m");
    
    DiskDriver disk;
    SimpleFS sfs;
    int disk_has_just_been_created = 0;

    DiskDriver_init(&disk, "disk.txt", 1024);
    printf("Disk has been created(SUCCESS)\n");

    DirectoryHandle* dir_root = SimpleFS_init(&sfs, &disk);
    if(dir_root == NULL){
        disk_has_just_been_created = 1;
        printf("Perhaps i need to format(i'll write something to read)...\n");
        SimpleFS_format(&sfs);
        dir_root = SimpleFS_init(&sfs, &disk);
        if(dir_root == NULL){
            printf("Problems in init disk(ERROR)\n\n");
            return -1;
        }
    }
    printf("\n");

    if(disk_has_just_been_created == 1){

        printf("\e[1;32mTESTS FOR SimpleFS_createFile\n\n\e[00m");
        
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

        printf("Creo un quarto file chiamato third_file\n");
        filehandle = SimpleFS_createFile(dir_root, "third_file");
        if(filehandle != NULL){
            printf("FileHandle creato correttamente(SUCCESS)\n\n");
            printf("Nome del file creato: %s\n\n", filehandle ->fcb ->fcb.name);
            SimpleFS_close(filehandle);
        }
        else{
            printf("FileHandle non creato perchè il file è già esistente(ERROR)\n\n");
        }

        printf("\n\n");

        printf("\e[1;32mTESTS FOR SimpleFS_openFile\n\n\e[00m");

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

        printf("Provo ad aprie il file 'third_file' nella directory root\n");
        file_to_open = SimpleFS_openFile(dir_root, "third_file");
        if(file_to_open == NULL){
            printf("Error in opening file 'third_file'(ERROR)\n\n");
        }
        else{
            printf("File 'third_file' aperto (SUCCESS)\n");
            printf("Nome del file aperto -> %s\n\n\n", file_to_open ->fcb ->fcb.name);
            SimpleFS_close(file_to_open);
        }
        
        printf("\n\n");

        printf("\e[1;32mTESTS FOR SimpleFS_readDir\n\n\e[00m");

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

        
        printf("\e[1;32mTESTS FOR SimpleFS_mkDir\n\n\e[00m");
        
        printf("Creo una sottocartella per la directory 'dir_root' chiamata 'subDir'\n");
        int res = SimpleFS_mkDir(dir_root, "subDir");
        if(res == -1){
            printf("Errore nella creazione della new_directory ' subDir ' \n");
            res = SimpleFS_mkDir(dir_root, "subDir");
            if(res == -1){
                printf("it's impossible to create new directory (ERROR)\n\n");
            }
        }
        else{
            printf("Directory creata con successo (SUCCESS) \n\n");
        }

        printf("Creo una sottocartella per la directory 'dir_root' chiamata 'subDirTry'\n");
        res = SimpleFS_mkDir(dir_root, "subDirTry");
        if(res == -1){
            printf("Errore nella creazione della new_directory ' subDirTry ' \n");
            res = SimpleFS_mkDir(dir_root, "subDirTry");
            if(res == -1){
                printf("it's impossible to create new directory (ERROR)\n\n");
            }
        }
        else{
            printf("Directory creata con successo (SUCCESS) \n\n");
        }
        
        
        printf("\e[1;32mTESTS FOR SimpleFS_changeDir\n\n\e[00m");
        printf("Provo ad andare dalla directory root alla directory subDir2(non esiste)\n");

        res = SimpleFS_changeDir(dir_root, "subDirTwo");
        if(res == -1){
            printf("sub-directory non trovata(SUCCESS)\n\n");
        }
        else{
            printf("Sub-directory trovata(ERROR)\n");
            printf("Directory corrente: %s\n", dir_root ->dcb ->fcb.name);
        }

        printf("Provo ad andare dalla directory root alla directory subDir(esiste)\n");
        res = SimpleFS_changeDir(dir_root, "subDir");
        if(res == -1){
            printf("sub-directory non trovata(ERROR)\n");
        }
        else{
            printf("Sub-directory trovata(SUCCESS)\n");
            printf("Directory corrente: %s\n\n", dir_root ->dcb ->fcb.name);
        }
        free(dir_root ->dcb);      //se non funziona cancellare qui

        printf("Provo ad andare alla directory padre di subDir (figlia della radice)\n");
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

        
        printf("\e[1;32mTESTS for SimpleFS_write\n\n\e[00m");

        printf("Provo a scrivere in un nuovo file 'write_file' una stringa abbastanza lunga\n");
        filehandle = SimpleFS_createFile(dir_root, "write_file");
        if(filehandle == NULL){
            printf("Error in creating a file(ERROR)\n");
        }
        else{
            printf("FileHandle creato correttamente(SUCCESS)\n");
            printf("Nome del file creato: %s\n", filehandle ->fcb ->fcb.name);
        }

        char* frase = (char*)malloc(sizeof(char) * 30);
        for(i = 0; i < 30; i++){
            frase[i] = 'a';
        }
        
        int bytes = SimpleFS_write(filehandle, frase, 30);
        if(bytes != -1){
            printf("Il file è stato scritto -> posizione nel file(quanti byte in totale): %d -> dati: %s (SUCCESS)\n\n", filehandle ->pos_in_file, filehandle ->fcb ->data);
        }
        else{
            printf("Errore nella scrittura del file write_file(ERROR)\n");
        }
        free(frase);

        printf("Provo a scrivere in un nuovo file 'write_file2' una stringa abbastanza lunga\n");
        FileHandle* filehandleW = SimpleFS_createFile(dir_root, "write_file2");
        if(filehandleW == NULL){
            printf("Error in creating a file(ERROR)\n");
        }
        else{
            printf("FileHandle creato correttamente(SUCCESS)\n");
            printf("Nome del file creato: %s\n", filehandleW ->fcb ->fcb.name);
        }

        frase = (char*)malloc(sizeof(char) * 500);
        for(i = 0; i < 499; i++){
            frase[i] = 'b';
        }
        frase[499] = 'c';

        bytes = SimpleFS_write(filehandleW, frase, 500);
        if(bytes != -1){
            printf("Il file è stato scritto -> posizione nel file(quanti byte in totale): %d -> dati(quelli che sono entrati in uno...) : %s (SUCCESS)\n\n", filehandleW ->pos_in_file, filehandleW ->fcb ->data);
        }
        else{
            printf("Errore nella scrittura del file write_file(ERROR)\n");
        }
        free(frase);

        //da qui la mia aggiunta per SimpleFS_write

        printf("Provo a scrivere altri due byte (d, e) nel file ' write_file2 '\n");
        frase = (char*)malloc(sizeof(char) * 2);
        frase[0] = 'd';
        frase[1] = 'e';

        bytes = SimpleFS_write(filehandleW, frase, 2);
        if(bytes != -1){
            printf("Il file è stato scritto -> posizione nel file(quanti byte in totale): %d -> dati(il blocco di prima...) : %s -> byte scritti: %d (SUCCESS)\n\n", filehandleW ->pos_in_file, filehandleW ->fcb ->data, bytes);
        }
        else{
            printf("Errore nella scrittura del file write_file(ERROR)\n");
        }
        free(frase);

        //fine mia aggiunta per SimpleFS_write

        printf("\e[1;32mTESTS for SimpleFS_read\n\n\e[00m");

        printf("Provo a leggere il file ' write_file ' appena scritto\n");

        frase = calloc(30, sizeof(char));

        bytes = SimpleFS_read(filehandle, frase, 30);

        if(bytes != -1){
            printf("Il file %s è stato letto -> dati TOTALI: %s (SUCCESS) \n\n", filehandle ->fcb ->fcb.name, frase);
        }
        else{
            printf("Errore nella lettura del file %s (ERROR) \n\n", filehandle ->fcb ->fcb.name);
        }
        free(frase);
        
        printf("Provo a leggere il file ' write_file2 ' appena scritto\n\n");
        
        //
        
        frase = calloc(502, sizeof(char));

        bytes = SimpleFS_read(filehandleW, frase, 502);
        
        //
        
        if(bytes != -1){
            printf("Il file %s è stato letto -> dati TOTALI: %s (SUCCESS) \n\n\n", filehandleW ->fcb ->fcb.name, frase);
        }
        else{
            printf("Errore nella lettura del file %s (ERROR) \n\n\n", filehandleW ->fcb ->fcb.name);
        }
        free(frase);
        
        
        printf("\e[1;32mTESTS for SimpleFS_remove\n\n\e[00m");

        printf("Provo a eliminare il file ' file_write3 ' (non esiste) \n");
        ret = SimpleFS_remove(dir_root, "write_file3");
        if(ret == -1){
            printf("Imposibile eliminare il file write_file3 (SUCCESS) \n\n");
        }
        else{
            printf("Il file write_file3 è stato eliminato (ERROR)\n\n");
        }

        printf("Provo a eliminare il file ' second_file ' (esiste) \n");
        ret = SimpleFS_remove(dir_root, "second_file");
        if(ret == -1){
            printf("Imposibile eliminare il file ' second_file ' (ERROR) \n\n");
        }
        else{
            printf("Il file ' second_file ' è stato eliminato (SUCCESS)\n\n");
        }

        printf("Provo a vedere se effettivamente il file ' second_file ' è stato eliminato provando ad aprirlo...\n");
        FileHandle* to_check_if_is_deleted = SimpleFS_openFile(dir_root, "second_file");
        if(to_check_if_is_deleted == NULL){
            printf("Impossibile aprire il file ' second_file ' (SUCCESS) \n\n");
        }
        else{
            printf("File 'second_file' aperto (ERROR)\n");
            printf("Nome del file aperto -> %s\n\n", to_check_if_is_deleted ->fcb ->fcb.name);
            SimpleFS_close(to_check_if_is_deleted);
        }

        printf("Leggo tutti i file a partire dalla radice per cercare di trovare il file eliminato....");
        names =(char**)malloc(sizeof(char*) * dir_root ->dcb ->num_entries);
        ret = SimpleFS_readDir(names, dir_root);
        if(ret == -1){
            printf("Impossibile leggere la directory root (ERROR) \n\n\n");
        }
        else{
            printf("\nLettura della directory avvenuta\n");
            printf("File trovati: ");
            for(i = 0; i < dir_root ->dcb ->num_entries; i++){
                printf("%s - ", names[i]);
            }
            printf("(SUCCESS)\n\n");
        }
        for(i = 0; i < dir_root ->dcb ->num_entries; i++){
            free(names[i]);
        }
        free(names);

        printf("Provo a eliminare la directory ' subDir2 ' (non esiste)\n");
        ret = SimpleFS_remove(dir_root, "subDir2");
        if(ret == -1){
            printf("Impossibile eliminare la directory ' subDir2 ' (SUCCESS)\n\n\n");
        }    
        else{ 
            printf("La directory ' subDir2 ' è stata eliminata (ERROR)\n\n\n");
        }

        printf("Provo a eliminare la directory ' subDirTry ' (esiste)\n");
        ret = SimpleFS_remove(dir_root, "subDirTry");
        if(ret == -1){
            printf("Impossibile eliminare la directory ' subDirTry ' (ERROR)\n\n\n");
        }    
        else{ 
            printf("La directory ' subDirTry ' è stata eliminata (SUCCESS)\n\n\n");
        }
        

        printf("Leggo tutti i file a partire dalla radice per cercare di trovare la directory eliminata....");
        names =(char**)malloc(sizeof(char*) * dir_root ->dcb ->num_entries);
        ret = SimpleFS_readDir(names, dir_root);
        if(ret == -1){
            printf("Impossibile leggere la directory root (ERROR) \n\n\n");
        }
        else{
            printf("\nLettura della directory avvenuta\n");
            printf("File trovati: ");
            for(i = 0; i < dir_root ->dcb ->num_entries; i++){
                printf("%s - ", names[i]);
            }
            printf("(SUCCESS)\n\n");
        }
        for(i = 0; i < dir_root ->dcb ->num_entries; i++){
            free(names[i]);
        }
        free(names);

        SimpleFS_close(filehandle);
        SimpleFS_close(filehandleW);
        free(dir_root ->dcb);
        free(dir_root);
    }
    else{
        
        printf("\e[1;32mQUESTO E' IL CASO IN CUI IL DISCO E' GIA' STATO CREATO\n\n\e[00m");
        
        printf("Nome della root: %s\n\n", dir_root ->dcb ->fcb.name);

        char** names =(char**)malloc(sizeof(char*) * dir_root ->dcb ->num_entries);
        int ret, i;

        ret = SimpleFS_readDir(names, dir_root);
        if(ret == -1){
            printf("Errore nella lettura della directory (ERROR)\n");
        }
        else{
            printf("Lettura della root avvenuta\n");
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

        
        printf("Provo ad aprire il file write_file2 dal disco\n");
        FileHandle* file_to_open = SimpleFS_openFile(dir_root, "write_file2");            
        if(file_to_open == NULL){
            printf("Errore nell'apertura del file ' write_file2 ' (ERROR)\n\n\n");
        }
        else{
            printf("File aperto, nome del file %s (SUCCESS)\n\n", file_to_open ->fcb ->fcb.name);    
        }
        
        printf("Provo a leggere il file write_file2 appena aperto\n");
        
        char* frase = calloc(502, sizeof(char));
        int bytes = SimpleFS_read(file_to_open, frase, 502);
        
        if(bytes == -1){
            printf("Errore nella lettura del file write_file2 (ERROR)\n\n\n");
        }
        else{
            printf("DATI OTTENUTI: %s (SUCCESS)\n\n", frase);
        }
        free(frase);
        

        printf("Creo una nuova cartella chiamata ' DirOne '\n");
        ret = SimpleFS_mkDir(dir_root, "DirOne");
        if(ret == -1){
            printf("Errore nella creazione della nuova directory (ERROR/SUCCESS)\n");
        }
        else{
            printf("Directory creata con successo\n");    
        }

        printf("Lista Files/Directories a partire dalla root\n");

        names =(char**)malloc(sizeof(char*) * dir_root ->dcb ->num_entries);
        ret = SimpleFS_readDir(names, dir_root);
        if(ret == -1){
            printf("Errore nella lettura della directory (ERROR)\n");
        }
        else{
            printf("Lettura della root avvenuta\n");
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

        
        printf("Faccio side-effect sulla directory ' DirOne '\n");
        ret = SimpleFS_changeDir(dir_root, "DirOne");
        if(ret == -1){
            printf("Errore nel cambio di cartella (ERROR) \n\n");
        }
        else{
            printf("Cambio di cartella avvenuto con successo\n");
            printf("Nome della cartella: %s (SUCCESS)\n\n", dir_root ->dcb ->fcb.name);
        }

        printf("Creo nella cartella ' DirOne ' un file chiamato ' File4DirOne ' \n");
        FileHandle* file_to_create = SimpleFS_createFile(dir_root, "File4DirOne");
        if(file_to_create == NULL){
            printf("Errore nella crezione del file ' File4Dir1 ' (ERROR)\n\n");
        }
        else{
            printf("File creato con successo\n");
            printf("Nome del file: %s\n\n", file_to_create ->fcb ->fcb.name);
            SimpleFS_close(file_to_create);
        }

        printf("Creo nella cartella ' DirOne ' un file chiamato ' File4DirTwo ' \n");
        file_to_create = SimpleFS_createFile(dir_root, "File4DirTwo");
        if(file_to_create == NULL){
            printf("Errore nella crezione del file ' File4DirTwo ' (ERROR)\n\n");
        }
        else{
            printf("File creato con successo\n");
            printf("Nome del file: %s\n\n", file_to_create ->fcb ->fcb.name);
            SimpleFS_close(file_to_create);
        }

        printf("Creo nella cartella ' DirOne ' un file chiamato ' File4DirThr ' \n");
        file_to_create = SimpleFS_createFile(dir_root, "File4DirThr");
        if(file_to_create == NULL){
            printf("Errore nella crezione del file ' File4DirThr ' (ERROR)\n\n");
        }
        else{
            printf("File creato con successo\n");
            printf("Nome del file: %s\n\n", file_to_create ->fcb ->fcb.name);
            SimpleFS_close(file_to_create);
        }

        
        printf("Creo nella cartella ' DirOne ' un file chiamato ' File4Dir4 ' \n");
        file_to_create = SimpleFS_createFile(dir_root, "File4Dir4");
        if(file_to_create == NULL){
            printf("Errore nella crezione del file ' File4Dir4 ' (ERROR)\n\n");
        }
        else{
            printf("File creato con successo\n");
            printf("Nome del file: %s\n\n", file_to_create ->fcb ->fcb.name);
        }

        printf("Lista files della directory ' DirOne '\n");

        names =(char**)malloc(sizeof(char*) * dir_root ->dcb ->num_entries);
        ret = SimpleFS_readDir(names, dir_root);
        if(ret == -1){
            printf("Errore nella lettura della directory ' DirOne ' (ERROR)\n");
        }
        else{
            printf("Lettura della directory ' DirOne ' avvenuta\n");
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

        free(dir_root ->dcb);       //se non funziona problema qui

        printf("Lista files della directory ' root ' \n");
        ret = SimpleFS_changeDir(dir_root, "..");
        if(ret == -1){
            printf("Errore nel cambio di cartella (ERROR) \n");
        }
        else{
            printf("Cambio di cartella avvenuto con successo\n");
            printf("Nome della cartella: %s \n", dir_root ->dcb ->fcb.name);
        }
        names = (char**)malloc(sizeof(char*) * dir_root ->dcb ->num_entries);
        ret = SimpleFS_readDir(names, dir_root);
        if(ret == -1){
            printf("Errore nella lettura della directory root\n\n");
        }
        else{
            printf("Lettura della directory ' root ' avvenuta\n");
            printf("File trovati: ");
            for(i = 0; i < dir_root ->dcb ->num_entries; i++){
                printf("%s - ", names[i]);
            }
        }
        printf("(SUCCESS)");
        for(i = 0; i < dir_root ->dcb ->num_entries; i++){
            free(names[i]);
        }
        free(names);
        printf("\n\n\n");

        printf("Provo a eliminare la directory DirOne\n");
        ret = SimpleFS_remove(dir_root, "DirOne");
        if(ret == -1){
            printf("Errore nel tentativo di eliminare la directory ' DirOne ' \n\n");
        }
        else{
            printf("Ok\n\n");
        }

        printf("Provo a creare una directory chiamata ' Dir4DirOne ' dentro la directory ' DirOne '\n");
        ret = SimpleFS_changeDir(dir_root, "DirOne");
        if(ret == -1){
            printf("Errore nel cambio di cartella (ERROR)\n\n\n");
        }
        else{
            printf("Cambio di cartella avvenuta con successo\n");
            printf("Nome della cartella: %s (SUCCESS)\n", dir_root ->dcb ->fcb.name);
        }
        
        ret = SimpleFS_mkDir(dir_root, "Dir4DirOne");
        if(ret == -1){
            printf("Impossibile creare la cartella ' Dir4DirOne ' (ERROR)\n\n\n");
        }
        else{
            printf("Directory creata con successo\n\n\n");
        }

        printf("Creo due file dentro la directory ' Dir4DirOne ', 'file1' e 'file2'\n");
        ret = SimpleFS_changeDir(dir_root, "Dir4DirOne");
        if(ret == -1){
            printf("Errore nel cambio di cartella (ERROR)\n");
        }
        else{
            printf("Cambio di cartella avvenuto con successo\n");
            printf("Nome della cartella: %s (SUCCESS)\n", dir_root ->dcb ->fcb.name);
        }
        
        FileHandle* file_to_write = SimpleFS_createFile(dir_root, "file1");
        if(file_to_write == NULL){
            printf("Errore nella creazione del file 'file1' (ERROR)\n");
        }
        else{
            printf("File creato con successo\n");
            printf("Nome del file: %s (SUCCESS)\n", file_to_write ->fcb ->fcb.name);
            SimpleFS_close(file_to_write);
        }

        file_to_write = SimpleFS_createFile(dir_root, "file2");
        if(file_to_write == NULL){
            printf("Errore nella creazione del file 'file1' (ERROR)\n\n");
        }
        else{
            printf("File creato con successo\n");
            printf("Nome del file: %s (SUCCESS)\n\n", file_to_write ->fcb ->fcb.name);
        }

        names = (char**)malloc(sizeof(char*) * dir_root ->dcb ->num_entries);
        ret = SimpleFS_readDir(names, dir_root);
        if(ret == -1){
            printf("Errore nella lettura della directory ' Dir4DirOne '\n");
        }
        else{
            printf("Lettura della directory ' root ' avvenuta\n");
            printf("File trovati: ");
            for(i = 0; i < dir_root ->dcb ->num_entries; i++){
                printf("%s - ", names[i]);
            }
        }
        printf("(SUCCESS)");
        for(i = 0; i < dir_root ->dcb ->num_entries; i++){
            free(names[i]);
        }
        free(names);
        printf("\n\n");

        frase = (char*)malloc(sizeof(char) * 600);
        for(i = 0; i < 599; i++){
            frase[i] = 'f';
        }
        frase[599] = 'g';
        
        printf("Provo a scrivere dentro a 'file2' qualcosa\n");
        ret = SimpleFS_write(file_to_write, frase, 600);
        if(ret == -1){
            printf("Errore nella scrittura del file (ERROR) \n\n\n");
        }
        else{
            printf("Il file è stato scritto -> posizione nel file(quanti byte in totale): %d -> dati(prima parte) : %s -> (SUCCESS)\n\n", file_to_write ->pos_in_file, file_to_write ->fcb ->data);
        }
        free(frase);

        
        printf("Provo a leggere 'file2' completo\n");
        frase = (char*)malloc(sizeof(char) * 600);
        ret = SimpleFS_read(file_to_write, frase, 600);
        if(ret == -1){
            printf("Errore nella lettura del file 'file_to_write'\n\n");
        }
        else{
            printf("Dati letti: %s -> (SUCCESS)\n\n", frase);
        }
        free(frase);

        free(dir_root ->dcb);

        printf("Ritorno alla directory radice\n");
        ret = SimpleFS_changeDir(dir_root, "..");
        if(ret == -1){
            printf("Errore nel cambio di directory\n");
        }
        else{
            printf("Cambio directory avvenuto\n");
            printf("Nome directory: %s\n", dir_root ->dcb ->fcb.name);
        }

        free(dir_root ->dcb);
        
        ret = SimpleFS_changeDir(dir_root, "..");
        if(ret == -1){
            printf("Errore nel cambio di directory\n\n\n");
        }
        else{
            printf("Cambio directory avvenuto\n");
            printf("Nome directory: %s\n\n\n", dir_root ->dcb ->fcb.name);
        }        
        
        SimpleFS_close(file_to_write);
        SimpleFS_close(file_to_open);
        SimpleFS_close(file_to_create);
        free(dir_root ->dcb);
        free(dir_root ->directory);
        free(dir_root);
    }
    
    return 0;
}
