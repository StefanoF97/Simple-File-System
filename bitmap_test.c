#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"

int main(int argc, char*argv[]){
    
    printf("\n");

    //Some tests for Bitmap_Block_ToIndex
    BitMapEntryKey bm1= BitMap_blockToIndex(38);
    BitMapEntryKey bm2= BitMap_blockToIndex(50);
    BitMapEntryKey bm3= BitMap_blockToIndex(150);
    
    printf("bm1 -> 38: %d %d\n", bm1.entry_num, bm1.bit_num);
    printf("bm2 -> 50: %d %d\n", bm2.entry_num, bm2.bit_num);
    printf("bm3 -> 150: %d %d\n", bm3.entry_num, bm3.bit_num);
    printf("\n");

    //Some tests for Bitmap_indexToBlock
    int one = BitMap_indexToBlock(bm1.entry_num, bm1.bit_num);
    int two = BitMap_indexToBlock(bm2.entry_num, bm2.bit_num);
    int three = BitMap_indexToBlock(bm3.entry_num, bm3.bit_num);
    
    printf("bm1: %d\n", one);
    printf("bm2: %d\n", two);
    printf("bm3: %d\n", three);
    
    printf("\n");

    //Some tests for Bitmap_get
    BitMap* bitmap1 = (BitMap*)malloc(sizeof(BitMap));
    bitmap1 ->num_bits = 8;
    bitmap1 ->entries = (char*)malloc(sizeof(char));
    bitmap1 ->entries[0] = 255;
    int first = BitMap_get(bitmap1, 0, 0);
    int second = BitMap_get(bitmap1, 0, 1);
    
    printf("11111111\n");
    printf("start = 0 status = 0 -> %d (Ok se -1)\n", first);
    printf("start = 0 status = 1 -> %d (Ok se 0)\n", second);
    printf("\n");

    BitMap* bitmap2 = (BitMap*)malloc(sizeof(BitMap));
    bitmap2 ->num_bits = 32;
    bitmap2 ->entries = (char*)malloc(sizeof(char) * 4);
    bitmap2 ->entries[0] = 0;
    bitmap2 ->entries[1] = 0;
    bitmap2 ->entries[2] = 0;
    bitmap2 ->entries[3] = 255;
    int third = BitMap_get(bitmap2, 1, 1);
    int fourth = BitMap_get(bitmap2, 0, 0);
    
    printf("00000000 00000000 00000000 11111111\n");
    printf("start = 1 status = 1 -> %d (Ok se 24)\n", third);
    printf("start = 0 status = 1 -> %d (Ok se 0)\n", fourth);
    printf("\n");

    free(bitmap2 ->entries);
    free(bitmap1 ->entries);
    free(bitmap1);
    free(bitmap2);

    //Some tests for Bitmap_test
    BitMap* bitmap3 = (BitMap*)malloc(sizeof(BitMap));
    bitmap3 ->num_bits = 8;
    bitmap3 ->entries = (char*)malloc(sizeof(char));
    bitmap3 ->entries[0] = 255;

    char a = bitmap3 ->entries[0];
    int i;
    for (i = 0; i < 8; i++) {
        printf("%d", !!((a << i) & 0x80));
    }
    printf("\n");

    int res = BitMap_set(bitmap3, 0, 0);
    printf("%d\n", res);
    
    a = bitmap3 ->entries[0];
    for (i = 0; i < 8; i++) {
        printf("%d", !!((a << i) & 0x80));
    }
    printf("\n");

    free(bitmap3 ->entries);
    free(bitmap3);

    return 0;
}