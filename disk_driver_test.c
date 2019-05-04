#include <stdio.h>
#include <stdlib.h>
#include "disk_driver.h"

int main(int argc, char* argv[]){
    
    DiskDriver* diskdriver = (DiskDriver*)malloc(sizeof(DiskDriver));
    const char* filename = "./disk.txt";

    DiskDriver_init(diskdriver, filename, 8);

    free(diskdriver);

    return 0;
}