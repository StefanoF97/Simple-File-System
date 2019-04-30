#include <math.h>
#include "bitmap.h"

BitMapEntryKey BitMap_blockToIndex(int num){
    
    BitMapEntryKey bmapentry;
    bmapentry.entry_num = num / 8;   //how many bytes?
    bmapentry.bit_num = num - (8 * (num / 8));   //offset
    return bmapentry;
}

int BitMap_indexToBlock(int entry, uint8_t bit_num){
    
    if(entry > 0 && bit_num > 0)
        return entry * 8 + bit_num;
    else
        return -1; //valori passati alla funzioni errati perchè negativi    
}

int BitMap_get(BitMap* bmap, int start, int status){

    if(start > (bmap -> num_bits))
        return -1;    //non avrò bit disponibili a partire da quel start

    BitMapEntryKey bmapentry;
    int bit;
    while(start < (bmap -> num_bits)){
        bmapentry = BitMap_blockToIndex(start);
        //da inserire il valore del bit, ipotesi shift verso destra di bit_num...
        if(bit == status)
            return start;
        
        start++;
    }
    
    return -1;
}



