#include "disk_driver.h"
#include "bitmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks){

    if(disk == NULL || filename == NULL || num_blocks < 0){
        printf("Parameters wrong\n");
        return;
    }

    int bmap_size = num_blocks / 8;    //dato che si fa in modo che ogni blocco abbia 8 bit, ho bisogno di sapere il numero di entry
    if (bmap_size % 8 != 0)            //inserisco eventualmente dei bit in più per creare almeno un blocco(o eventualmente per creare un blocco in più in caso num_blocks %8 != 0)
        bmap_size += 1;
    
    if (!access(filename, F_OK)){
        
        int fd = open(filename, O_RDWR, 0666);
        if(fd == -1){
            printf("Error in opening file!\n");
            return;
        }

        DiskHeader* header = (DiskHeader*)mmap(NULL, sizeof(DiskHeader) + bmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  //mappo sul file in ingresso memoria virtuale
        if(header == MAP_FAILED){
            printf("Mappatura su file non riuscita...ritentare\n");
            close(fd);
            return;
        }
        
        header ->num_blocks = num_blocks;
        header ->bitmap_blocks = num_blocks;
        header ->bitmap_entries = bmap_size;
        header ->first_free_block = 0;
        header ->free_blocks = num_blocks;
        
        disk ->header = header;
        disk ->bitmap_data = sizeof(DiskHeader) + (char*)header;
        disk ->fd = fd;

        int i = 0;
        while(i < strlen(disk -> bitmap_data)){    //inizializzo a zero bitmap_data
            disk ->bitmap_data[i] = 0;
            i++;
        }
        
    }
    else{
        return;
    }
}

int DiskDriver_readBlock(DiskDriver* disk, void* dest, int block_num){

    if(disk == NULL || disk ->header ->bitmap_blocks < block_num || block_num < 0){
        return -1;
    }

    BitMap bmap;                                    //sfrutto le funzioni che ho scritto per la BitMap
    bmap.entries = disk ->bitmap_data;
    bmap.num_bits = disk ->header ->bitmap_blocks;

    BitMapEntryKey bmapentry = BitMap_blockToIndex(block_num);
    if((bmap.entries[bmapentry.entry_num] >> (bmapentry.bit_num) & 0x01) == 0)
        return -1;   //non c'è nulla da leggere (è vuoto il blocco)

    //SEEK_SET = offset e' aggiunto dall'inizio del file -> per le funzioni di read/write
    //devo cercare nel fs dopo lo spazio occupato dalla bitmap (e dal DiskHeader).
    int offset = lseek(disk ->fd, sizeof(DiskHeader) + (disk ->header ->bitmap_entries) + (block_num * BLOCK_SIZE), SEEK_SET);  
    if(offset == -1){
        printf("lseek fallita\n");
        return;
    }

    int ret; 
    int read_bytes = 0;
    while(read_bytes < BLOCK_SIZE){         
        //grazie alla lseek sono già posizionato nel punto dove devo leggere 
        //da finire normale funzione di lettura
        ret = read(disk ->fd, dest + read_bytes, BLOCK_SIZE - read_bytes);

        if(ret == -1 && errno == EINTR)
            continue;
        else{
            printf("errore in lettura\n");
            return;
        }
        
        if(ret == 0)
            break;

        read_bytes += ret;
    }

    return 0;       //se ritorna 0 significa che tutto è andato bene
}

int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num){
    
}
