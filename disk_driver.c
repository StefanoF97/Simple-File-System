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
#include <sys/syscall.h>
#include <linux/falloc.h>

void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks){

    if(disk == NULL || filename == NULL || num_blocks < 0){
        printf("Parameters wrong\n");
        return;
    }

    int bmap_size = num_blocks / 8;    //dato che si fa in modo che ogni blocco abbia 8 bit, ho bisogno di sapere il numero di entry

    if(bmap_size == 0){     //when testing, bitmap_entries == 0 but it's an error...if num_blocks less than eight, anyway i need an entry(bmap_size = 1 necessary)
        bmap_size += 1;
    }

    int fd;
    if (!access(filename, F_OK)){
        
        fd = open(filename, O_RDWR, 0666);
        if(fd == -1){
            printf("Error in opening file!\n");
            return;
        }

        if(posix_fallocate(fd, 0, sizeof(DiskHeader)+bmap_size) != 0){        //ho dovuto inserire l'allocazione del file per evitare l'errore "Errore di bus(core dump creato)"
            printf("Errore nella allocazione del file\n");
            return;
        }

        DiskHeader* header = (DiskHeader*)mmap(0, sizeof(DiskHeader) + bmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  //mappo sul file in ingresso memoria virtuale
        if(header == MAP_FAILED){
            printf("Mappatura su file non riuscita...ritentare\n");
            close(fd);
            return;
        }

        disk ->header = header;
        disk ->bitmap_data = (char*)header + sizeof(DiskHeader);
        disk ->fd = fd;
        
        header ->num_blocks = num_blocks;
        header ->bitmap_blocks = num_blocks;
        header ->bitmap_entries = bmap_size;
        header ->first_free_block = 0;
        header ->free_blocks = num_blocks;

        int i = 0;
        while(i < header ->bitmap_blocks){    //inizializzo a zero bitmap_data
            disk ->bitmap_data[i] = 0;
            i++;
        }
    }
    else{
        printf("File cannot be accessed\n");
        return;
    }
    
    printf("Init avvenuta correttamente\n");
}

int DiskDriver_readBlock(DiskDriver* disk, void* dest, int block_num){

    if(disk == NULL || disk ->header ->bitmap_blocks-1 < block_num || block_num < 0 || dest == NULL){
        printf("Parametri passati a DiskDriver_readBlock non conformi\n");
        return -1;
    }

    BitMap bmap;                                    //sfrutto le funzioni che ho scritto per la BitMap
    bmap.entries = disk ->bitmap_data;
    bmap.num_bits = disk ->header ->bitmap_blocks;

    BitMapEntryKey bmapentry = BitMap_blockToIndex(block_num);
    if((bmap.entries[bmapentry.entry_num] >> (bmapentry.bit_num) & 0x01) == 0){
        printf("Blocco da leggere vuoto, nulla da leggere\n");
        return -1;   
    }

    //SEEK_SET = offset e' aggiunto dall'inizio del file -> per le funzioni di read/write
    //devo cercare nel fs dopo lo spazio occupato dalla bitmap (e dal DiskHeader).
    int offset = lseek(disk ->fd, sizeof(DiskHeader) + (disk ->header ->bitmap_entries) + (block_num * BLOCK_SIZE), SEEK_SET);  
    if(offset == -1){
        printf("Errore in lseek (in DiskDriver_readBlock )\n");
        return -1;
    }

    int ret; 
    int read_bytes = 0;
    while(read_bytes < BLOCK_SIZE){         
        //grazie alla lseek sono già posizionato nel punto dove devo leggere 
        //da finire normale funzione di lettura
        ret = read(disk ->fd, dest + read_bytes, BLOCK_SIZE - read_bytes);

        if(ret == -1 && errno == EINTR)
            continue;
        
        if(ret == -1){
            printf("errore in lettura\n");
            return -1;
        }
        
        if(ret == 0)
            break;

        read_bytes += ret;
    }

    printf("Byte letti: %d\n", read_bytes);

    return 0;       //se ritorna 0 significa che tutto è andato bene
}

int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num){
    
    if(disk == NULL || disk ->header ->bitmap_blocks-1 < block_num || block_num < 0 || src == NULL){
        printf("parametri passati a DiskDriver_writeBlock non conformi\n");
        return -1;
    }

    BitMap bmap;
    bmap.entries = disk ->bitmap_data;
    bmap.num_bits = disk ->header ->bitmap_blocks;

    BitMapEntryKey bmapentry = BitMap_blockToIndex(block_num);
    if((bmap.entries[bmapentry.entry_num] >> (bmapentry.bit_num) & 0x01) == 1){
        printf("Blocco da scrivere occupato, impossibile sovrascriverlo\n");
        return -1;   
    }

    //come nella readBlock...sfrutto lseek per far "puntare" il file descriptor al blocco da scrivere
    int offset = lseek(disk ->fd, sizeof(DiskHeader) + (disk ->header ->bitmap_entries) + (block_num * BLOCK_SIZE), SEEK_SET);
    if(offset == -1){
        printf("Errore in lseek (in DiskDriver_writeBlock )\n");
        return -1;
    }

    //in questo spazio devo aggiornare sia il numero di blocchi liberi(va decrementato perchè sto scrivendo un blocco libero) e
    //sia inserire in first_free_block un nuovo indice;
    //settare a 1 la bitmap in posizione block_num

    disk ->header ->free_blocks -= 1;

    if(disk ->header ->first_free_block == block_num)   //in realtà, vado a modificare first_free_block solo se block_num è uguale a disk ->header ->first_free_block, 
                                                        //altrimenti lo lascio inalterato
        disk ->header ->first_free_block = DiskDriver_getFreeBlock(disk, block_num + 1);    //parto da block_num + 1 a cercare un nuovo blocco libero

    BitMap_set(&bmap, block_num, 1);

    int ret;
    int written_bytes = 0;
    while (written_bytes < BLOCK_SIZE){
        
        ret = write(disk ->fd, src + written_bytes, BLOCK_SIZE - written_bytes);

        if(ret == -1 && errno == EINTR)
            continue;
        
        if(ret == -1){
            printf("Errore in scrittura\n");
            return -1;
        }

        if(ret == 0)
            break;
        
        written_bytes += ret;
    }

    printf("Byte scritti: %d\n", written_bytes);
    
    return 0;   //se si ritorna 0 è andato tutto bene

}

int DiskDriver_getFreeBlock(DiskDriver* disk, int start){
    //posso sicuramente sfruttare la bitmap_get ponendo status = 0, in questo modo
    //mi restituirà la posizione del primo blocco che ha bit = status(0 in questo caso)

    if(disk == NULL || start < 0 || disk ->header ->bitmap_blocks < start){
        printf("Parametri passati a DiskDriver_getFreeBlock non conformi\n");
        return -1;
    }

    BitMap bmap;
    bmap.entries = disk ->bitmap_data;
    bmap.num_bits = disk ->header ->bitmap_blocks;

    int position = BitMap_get(&bmap, start, 0);

    return position;

}

int DiskDriver_flush(DiskDriver* disk){
    
    if(disk == NULL){
        printf("cannot flushing, Diskdriver is null\n");
        return -1;
    }

    int ret = msync(disk ->header, sizeof(DiskHeader) + disk ->header ->bitmap_entries, MS_SYNC);   //msynch flushes memory mmapped on header

    if(ret == -1){
        printf("Error in msync\n");
        return -1;
    }

    return 0;
}

int DiskDriver_freeBlock(DiskDriver* disk, int block_num){
    
    if(block_num > disk ->header ->bitmap_blocks || disk == NULL || block_num < 0){
        printf("Cannot free block because wrong parameters\n");
        return -1;
    }

    BitMap bmap;
    bmap.entries = disk ->bitmap_data;
    bmap.num_bits = disk ->header ->bitmap_blocks;

    int ret = BitMap_set(&bmap, block_num, 0);

    if(ret == -1){
        printf("Error in Bitmap_set when trying fo free block\n");
        return -1;
    }

    disk ->header ->free_blocks += 1;           //Added two necessary operations to update free block and to eventually set first_free_block

    if(disk ->header ->first_free_block > block_num)
        disk ->header ->first_free_block = block_num;
    
    return 0;
}



