#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;

void writeBlock(FILE* disk, int blockNum, char* data){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET); // Find the block to write to
    fwrite(data, strlen(data), 1, disk); 
    //to FIX the potential error that might caused by strlen(), please check examples like "3.createINodeDemo.c", which adds a new parameter "size"
}

int main() {
    FILE* disk = fopen("vdisk", "rb+"); // Open existing file to be read and written in binary mode
    writeBlock(disk, 2, "Hello world!");

    fclose(disk);
    return 0;
}
