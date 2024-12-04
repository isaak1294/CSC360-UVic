#include <stdio.h>
#include <stdlib.h>
const int BLOCK_SIZE = 512;
const int NUM_BLOCKS = 4096;

int main(int argc, char* argv[]) {
    FILE* disk = fopen("vdisk", "wb"); // Open the file to be written to in binary mode
    char* init=calloc(BLOCK_SIZE*NUM_BLOCKS, 1);
    fwrite(init, BLOCK_SIZE*NUM_BLOCKS, 1, disk);

    fclose(disk);
    return 0;
}
