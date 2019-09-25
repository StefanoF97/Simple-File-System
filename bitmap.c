#include "bitmap.h"
#include <string.h>
#include <math.h>
#define BITS 8
//#define NULL ((void*) 0)

BitMapEntryKey BitMap_blockToIndex(int num){
    
    BitMapEntryKey bmapentry;
    bmapentry.entry_num = num / BITS;   //quanti bytes?
    bmapentry.bit_num = (char)(num - ((num / BITS) * BITS));   //offset, come se facessi (char)(num % 8), per posizione del bit
    return bmapentry;
}

int BitMap_indexToBlock(int entry, uint8_t bit_num){
    
    if(entry > 0 && bit_num > 0)
        return entry * BITS + bit_num;
    else
        return -1; //valori passati alla funzioni errati perchè negativi    
}

int BitMap_get(BitMap* bmap, int start, int status){

    if(bmap == NULL || start > (bmap -> num_bits) || start < 0 || status < 0)
        return -1;    //non avrò bit disponibili a partire da quel start, oppure i valori passati non sono corenti
        

    BitMapEntryKey bmapentry;
    int bit;
    while(start < (bmap -> num_bits)){
        bmapentry = BitMap_blockToIndex(start);
        //da inserire il valore del bit, ipotesi shift verso destra di bit_num...
        bit = bmap -> entries[bmapentry.entry_num] >> (bmapentry.bit_num) & 0x01;   //ho un array di char, prendo la posizione dell'entry all'interno di quest'ultimo, shifto dell'offset e grazie alla maschera ottengo 0 o 1 a seconda del bit
        if(bit == status)
            return start;
        
        start++;
    }
    
    return -1;
}

int BitMap_set(BitMap* bmap, int pos, int status){
    
    /*
    if (bmap == NULL || pos < 0 || status < 0 || pos >= bmap ->num_bits)
        return -1; 

    BitMapEntryKey appobmap = BitMap_blockToIndex(pos);     //ricavo la entry nella posizione desiderata senza bisogno di un ciclo

    int mask = 1 << appobmap.bit_num;   //sfrutto il solito offset in modo da ottenere la posizione giusta del bit
    
    bmap ->entries[appobmap.entry_num] = (bmap ->entries[appobmap.entry_num] & ~mask) | ((status << pos) & mask);
    return abs((bmap ->entries[appobmap.entry_num] & ~mask) | ((status << pos) & mask));
    */

    if(pos >= bmap->num_bits) return -1;                                
    
    BitMapEntryKey bmapentrykey = BitMap_blockToIndex(pos);     //ricavo la entry nella posizione desiderata senza bisogno di un ciclo
                
    if(status == 1){        //sfrutto il solito offset in modo da ottenere la posizione giusta del bit tramite maschera
        bmap->entries[bmapentrykey.entry_num] = bmap->entries[bmapentrykey.entry_num] | 1 << bmapentrykey.bit_num; 
        return bmap->entries[bmapentrykey.entry_num] | 1 << bmapentrykey.bit_num;
    }
    else{
        bmap->entries[bmapentrykey.entry_num] = bmap->entries[bmapentrykey.entry_num] & (~(1 << bmapentrykey.bit_num));
        return bmap->entries[bmapentrykey.entry_num] & (~(1 << bmapentrykey.bit_num));
    }
}





