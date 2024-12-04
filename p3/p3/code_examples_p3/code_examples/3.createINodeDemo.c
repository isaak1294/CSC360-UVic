#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;
const int INODE_SIZE = 32;

void readBlock(FILE* disk, int blockNum, char* buffer){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, disk);
}

void writeBlock(FILE* disk, int blockNum, char* data, int size){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fwrite(data, size, 1, disk); 
}

char* createEmptyInode() {
    char* inode = (char*)malloc(32);
    short dataBlock1=3;
    memcpy(inode+8, &dataBlock1, 2); //First direct block value pointing to block number 3
    return inode;
}

int main(int argc, char* argv[]) {
    FILE* disk = fopen("vdisk", "rb+");
    char* myInode = createEmptyInode();
    // Maybe add more things to the inode
    writeBlock(disk, 2, myInode, 32);

    char* buffer = (char*)malloc(BLOCK_SIZE);
    readBlock(disk, 2, buffer);
    short temp;
    memcpy(&temp, buffer+8, 2);
    printf("%d\n", temp);

    free(myInode);
    fclose(disk);
    return 0;
}
