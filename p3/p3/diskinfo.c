#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint16_t block_size;
uint32_t block_count;
uint32_t fat_start;
uint32_t fat_blocks;
uint32_t root_dir_start;
uint32_t root_dir_blocks;

// Set first argument as file name
char *file_name;

void swapEndian16(uint16_t *val){
    // Shift 8 bits to the right and OR with 8 bits shifted to the left
    *val = (*val >> 8) | (*val << 8);
}

void swapEndian32(uint32_t *val){
    // Shift 24 bits to the right and OR with 8 bits shifted to the left and 8 bits shifted to the right
    *val = (*val >> 24) | ((*val << 8) & 0x00FF0000) | ((*val >> 8) & 0x0000FF00) | (*val << 24);
}

void readSuperBlockInfo(char *file_name){
    FILE *fp = fopen(file_name, "rb+");
    if(fp == NULL){
        printf("Error: Unable to open disk image\n");
        exit(1);
    }
    fseek(fp, 8, SEEK_SET);
    fread(&block_size, sizeof(uint16_t), 1, fp);
    swapEndian16(&block_size);
    fseek(fp, 10, SEEK_SET);
    fread(&block_count, sizeof(uint32_t), 1, fp);
    swapEndian32(&block_count);
    fseek(fp, 14, SEEK_SET);
    fread(&fat_start, sizeof(uint32_t), 1, fp);
    swapEndian32(&fat_start);
    fseek(fp, 18, SEEK_SET);
    fread(&fat_blocks, sizeof(uint32_t), 1, fp);
    swapEndian32(&fat_blocks);
    fseek(fp, 22, SEEK_SET);
    fread(&root_dir_start, sizeof(uint32_t), 1, fp);
    swapEndian32(&root_dir_start);
    fseek(fp, 26, SEEK_SET);
    fread(&root_dir_blocks, sizeof(uint32_t), 1, fp);
    swapEndian32(&root_dir_blocks);
}

void readFATInfo(char *file_name){
    FILE *fp = fopen(file_name, "rb+");
    if(fp == NULL){
        printf("Error: Unable to open disk image\n");
        exit(1);
    }
    fseek(fp, fat_start * block_size, SEEK_SET);
    const int TABLE_SIZE = fat_blocks * block_size / 4;
    uint32_t *fat_table = (uint32_t *)malloc(TABLE_SIZE * sizeof(uint32_t));
    if (fat_table == NULL) {
        printf("Error: Unable to allocate memory for FAT table\n");
        fclose(fp);
        exit(1);
    }
    fread(fat_table, sizeof(uint32_t), fat_blocks * block_size / 4, fp);
    for(int i = 0; i < fat_blocks * block_size / 4; i++){
        swapEndian32(&fat_table[i]);
    }
    int free_blocks = 0;
    int reserved_blocks = 0;
    int allocated_blocks = 0;
    for(int i = 0; i < fat_blocks * block_size / 4; i++){
        if(fat_table[i] == 0){
            free_blocks++;
        }else if(fat_table[i] == 1){
            reserved_blocks++;
        }else{
            allocated_blocks++;
        }
    }
    free(fat_table);
    printf("\n");
    printf("FAT Information\n");
    printf("Free Blocks: %d\n", free_blocks);
    printf("Reserved Blocks: %d\n", reserved_blocks);
    printf("Allocated Blocks: %d\n", allocated_blocks);
}

void printSuperBlockInfo(){
    printf("Super Block Information\n");
    printf("Block size: %d\n", block_size);
    printf("Block count: %d\n", block_count);
    printf("FAT starts: %d\n", fat_start);
    printf("FAT blocks: %d\n", fat_blocks);
    printf("Root directory start: %d\n", root_dir_start);
    printf("Root directory blocks: %d\n", root_dir_blocks);

    printf("\n");
    readFATInfo(file_name);
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Error: Invalid number of arguments\n");
        exit(1);
    }
    file_name = argv[1];
    readSuperBlockInfo(file_name);
    printSuperBlockInfo();
    return 0;
}