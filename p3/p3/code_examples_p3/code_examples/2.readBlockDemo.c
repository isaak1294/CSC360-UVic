#include <stdio.h>
#include <stdlib.h>
const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;

void readBlock(FILE* disk, int blockNum, char* buffer){
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, disk);
}




int main(int argc, char* argv[]) {
    FILE* disk = fopen("vdisk", "rb+"); // Notice that we are now reading and writing
    char* buffer = (char*)malloc(BLOCK_SIZE);
    readBlock(disk, 2, buffer);
//    printf("%s", buffer);
    int i;
    for (i = 0; i < BLOCK_SIZE; i++){
        printf("%2x ", buffer[i]);
    }
    printf("\n");
    free(buffer);
    fclose(disk);
    return 0;
}
