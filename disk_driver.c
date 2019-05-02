#include "disk_driver.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks){

    if(disk == NULL || filename == NULL || num_blocks < 0){
        printf("Parameters wrong\n");
        return;
    }

    int bmap_size = num_blocks / 8;    //dato che si fa in modo che ogni blocco abbia 8 bit, ho bisogno di sapere il numero di entry
    if (bmap_size % 8 != 0)            //inserisco eventualmente dei bit in più per creare almeno un blocco
        bmap_size += 1;
    
    if (access(filename, F_OK)){
        
        int fd = open(filename, O_RDWR, 0666);
        if(fd == -1){
            printf("Error in opening file!\n");
            return;
        }

        DiskHeader* header = (DiskHeader*)mmap(0, sizeof(DiskHeader) + bmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  //mappo sul file in ingresso memoria virtuale
        if(header == MAP_FAILED){
            printf("Mappatura su file non riuscita...ritentare\n");
            close(fd);
            return;
        }

        disk ->header = header;
        disk ->bitmap_data = sizeof(DiskHeader) + (char*)header;
        disk ->fd = fd;
    }
    else{
        //caso in cui il file non è accessibile...dovrò aprirlo io e ri-mmappare
    }
    
    
    
}