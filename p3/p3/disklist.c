#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

const int BLOCK_SIZE = 512;
char* filepath;
char type;
int size;
char name[31];
char date[11];
char time[6];
int i;

//Structs copied from lab:
struct __attribute__((__packed__)) superblock_t {
    uint8_t fs_id[8];
    uint16_t block_size;
    uint32_t file_system_block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

struct __attribute__((__packed__)) dir_entry_timedate_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct __attribute__((__packed__)) dir_entry_t {
    uint8_t status;
    uint32_t starting_block;
    uint32_t block_count;
    uint32_t size;
    struct dir_entry_timedate_t create_time;
    struct dir_entry_timedate_t modify_time;
    uint8_t filename[31];
    uint8_t unused[6];
};

void swapendian16(uint16_t *val){
    // Shift 8 bits to the right and OR with 8 bits shifted to the left
    *val = (*val >> 8) | (*val << 8);
}

void swapendian32(uint32_t *val){
    // Shift 24 bits to the right and OR with 8 bits shifted to the left and 8 bits shifted to the right
    *val = (*val >> 24) | ((*val << 8) & 0x00FF0000) | ((*val >> 8) & 0x0000FF00) | (*val << 24);
}

uint16_t se16(uint16_t val) {
    return (val >> 8) | (val << 8);
}

uint32_t se32(uint32_t val) {
    return (val >> 24) | ((val << 8) & 0x00FF0000) | ((val >> 8) & 0x0000FF00) | (val << 24);
}

void readBlock(FILE* disk, int blockNum, char* buffer){
    //printf("Reading block %d\n", blockNum);
    fseek(disk, blockNum * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, disk);
}

void printInfo(struct dir_entry_t* working_dir){
    printf("%s ", working_dir->status == 0x03 ? "F" : "D");
    printf("%10d ", se32(working_dir->size));
    printf("%30s ", working_dir->filename);
    printf("%4d-%02d-%02d ", working_dir->modify_time.year, working_dir->modify_time.month, working_dir->modify_time.day);
    printf("%02d:%02d:%02d\n", working_dir->modify_time.hour, working_dir->modify_time.minute, working_dir->modify_time.second);
}

/*
Input format:
./disklist <disk image> <file path>

Output format:
<Type> <Size> <Name> <Date Modified> <Time Modified>
eg. D 0 . 2019-09-01 12:00:00
    F 100 file.txt 2019-09-01 12:00:00
*/


int main(int argc, char* argv[]){
    //Check arguments
    if(argc != 3){
        if(argc == 2){
            filepath = ".";
        }
        else{
            printf("Error: Invalid number of arguments\n  Expected: disklist <disk image> <file path>\n");
            exit(1);
        }
    }

    //Open disk image
    FILE *fp = fopen(argv[1], "rb+");
    int fd = open(argv[1], O_RDWR);
    if(fp == NULL){
        printf("Error: Unable to open disk image\n");
        exit(1);
    }

    //Read block 0
    

    struct stat buffer;
    if(stat(argv[1], &buffer)){
        printf("Error: Unable to read disk image\n");
        exit(1);
    }
    off_t startingBlock = 0 * BLOCK_SIZE;
    char* addr = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED){
        printf("Error: Unable to map disk image\n");
        exit(1);
    }
    struct superblock_t* superblock = (struct superblock_t*)addr;
    addr = addr + se32(superblock->root_dir_start_block) * BLOCK_SIZE;
    char* path[128];
    int tokens = 0;
    if(argc == 3){
        char* token = strtok(argv[2], "/");
        int i = 0;
        tokens = 0;
        while(token != NULL){
            path[i] = token;
            tokens++;
            token = strtok(NULL, "/");
            i++;
        }
    }
    
    struct dir_entry_t* working_dir;
    //if the path is just the root directory, print the contents of the root directory
    if(argc == 2 || (argc == 3 && tokens == 0)){
        for(i = 0; i < BLOCK_SIZE; i+= 64){
            working_dir = (struct dir_entry_t*)(addr + i);
            if(working_dir->status == 0x00){
                continue;
            }
            printInfo(working_dir);
        }
        }else{
        //if the path is not the root directory, traverse the path
            for(i = 0; i < tokens; i++){
                int j;
                for(j = 0; j < BLOCK_SIZE; j+= 64){
                    working_dir = (struct dir_entry_t*)(addr + j);
                    if(working_dir->status == 0x00){
                        break;
                    }
                    if(strcmp(working_dir->filename, path[i]) == 0){
                        break;
                    }
                }
                if(working_dir->status == 0x00){
                    printf("Error: File not found\n");
                    exit(1);
                }
                if(i == tokens - 1){
                    printInfo(working_dir);
                }else{
                    addr = addr + working_dir->starting_block * superblock->block_size;
                    printf("%d %d\n", working_dir->starting_block, superblock->block_size);
                }
            }
        }
    munmap(addr, buffer.st_size);
    fclose(fp);
    return 0;
    
}