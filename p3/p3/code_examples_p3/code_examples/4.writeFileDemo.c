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
    fwrite(data, size, 1, disk); // Note: this will overwrite existing data in the block
}

char* createEmptyInode() {
    char* inode = malloc(32);
    short dataBlock1=3;
    memcpy(inode+8, &dataBlock1, 2);
    return inode;
}

void createFile(FILE* disk) {
    char* inode = createEmptyInode();
    // Add more things to inode?
    writeBlock(disk, 2, inode, 32);
    
    free(inode);
}

void writeToFile(FILE* disk, char* data, int size) {
    char* inodeBuffer = (char*)malloc(BLOCK_SIZE);
    readBlock(disk, 2, inodeBuffer);
    short fileBlockNumber;
    memcpy(&fileBlockNumber, inodeBuffer+8, 2);
    writeBlock(disk, fileBlockNumber, data, size);

    free(inodeBuffer);
}

int main(int argc, char* argv[]) {
    FILE* disk = fopen("vdisk", "rb+");
    createFile(disk);    
    writeToFile(disk, "Hello World!", 12);

    fclose(disk);
    return 0;
}
