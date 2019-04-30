#include <math.h>
#include "bitmap.h"

BitMapEntryKey BitMap_blockToIndex(int num){
    BitMapEntryKey bmap;
    bmap.entry_num = num / 8;   //how many bytes?
    bmap.bit_num = num - (8 * (num / 8));   //offset
    return bmap;
}

int BitMap_indexToBlock(int entry, uint8_t bit_num){
    if(entry > 0 && bit_num > 0)
        return entry * 8 + bit_num;
    else
        return -1; //valori passati alla funzioni errati perchè negativi    
}



