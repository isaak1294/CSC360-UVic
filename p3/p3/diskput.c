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

int tokenize(char* path, char* tokens[]){
    char* token = strtok(path, "/");
    int i = 0;
    while(token != NULL){
        tokens[i] = token;
        token = strtok(NULL, "/");
        i++;
    }
    return i;
}

//Check if file exists in current linux directory:
int fileExists(char* filename){
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

//Check if directory exists in disk:
int dirExists(char* path[128], int tokens, char* addr){
    int i;
    struct superblock_t* superblock = (struct superblock_t*)addr;
    addr = addr + se32(superblock->root_dir_start_block) * BLOCK_SIZE;
    struct dir_entry_t* working_dir;
    for(i = 0; i < tokens; i++){
                int j;
                for(j = 0; j < BLOCK_SIZE; j+= 64){
                    working_dir = (struct dir_entry_t*)(addr + j);
                    if(working_dir->status == 0x00){
                        break;
                    }
                    if(strcmp(working_dir->filename, path[i]) == 0){
                            return 1;
                    }

                }
                if(working_dir->status == 0x00){
                    printf("Directory not found\n");
                    return 0;
                }
                if(i != tokens - 1){
                    addr = addr + se32(working_dir->starting_block) * BLOCK_SIZE;
                }
            }
    return 0;
}

int diskHasSpace(char* addr, int size){
    struct superblock_t* superblock = (struct superblock_t*)addr;
    addr = addr + se32(superblock->root_dir_start_block) * BLOCK_SIZE;
    struct dir_entry_t* working_dir;
    for(i = 0; i < BLOCK_SIZE; i+= 64){
        working_dir = (struct dir_entry_t*)(addr + i);
        if(working_dir->status == 0x00){
            break;
        }
    }
    if(working_dir->status == 0x00){
        return 1;
    }
    return 0;
}

//Create new directory entry in the given path
struct dir_entry_t* makeDirEntry(char* path[128], int tokens, char* addr){
    int i;
    struct superblock_t* superblock = (struct superblock_t*)addr;
    addr = addr + se32(superblock->root_dir_start_block) * BLOCK_SIZE;
    struct dir_entry_t* working_dir;
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
                    break;
                }
                if(i != tokens - 1){
                    addr = addr + se32(working_dir->starting_block) * BLOCK_SIZE;
                }
            }
    if(working_dir->status == 0x00){
        working_dir->status = 0x03;
        working_dir->starting_block = 0;
        working_dir->block_count = 0;
        working_dir->size = 0;
        struct dir_entry_timedate_t* time = (struct dir_entry_timedate_t*)malloc(sizeof(struct dir_entry_timedate_t));
        working_dir->create_time = *time;
        working_dir->modify_time = *time;
        strcpy(working_dir->filename, path[tokens - 1]);
        return working_dir;
    }
    printf("Error: Directory already exists\n");
    exit(1);
}

void putfile(char* addr, char* src, struct dir_entry_t* destination){
    FILE* file = fopen(src, "r");
    if(file == NULL){
        printf("Error: Unable to open file\n");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if(size > 0){
        int blocks = size / BLOCK_SIZE;
        if(size % BLOCK_SIZE != 0){
            blocks++;
        }
        destination->size = size;
        destination->block_count = blocks;
        destination->starting_block = 0;
        int i;
        for(i = 0; i < blocks; i++){
            char* buffer = (char*)malloc(BLOCK_SIZE);
            fread(buffer, 1, BLOCK_SIZE, file);
            if(i == 0){
                destination->starting_block = i;
            }else{
                struct dir_entry_t* working_dir = destination;
                while(working_dir->starting_block != 0){
                    working_dir = (struct dir_entry_t*)(addr + se32(working_dir->starting_block) * BLOCK_SIZE);
                }
                working_dir->starting_block = i;
            }
            memcpy(addr + i * BLOCK_SIZE, buffer, BLOCK_SIZE);
            free(buffer);
        }
    }
    fclose(file);
}


int main(int argc, char* argv[]){

    if(argc != 4){
        printf("Error: Invalid number of arguments\n  Expected: diskget <disk image> <file source> <file destination>\n");
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);
    if(fd == -1){
        printf("Error: Unable to open disk image\n");
        exit(1);
    }
    struct stat buffer;
    if(stat(argv[1], &buffer)){
        printf("Error: Unable to read disk image\n");
        exit(1);
    }

    char* src = argv[2];
    char* dest = argv[3];

    int stokens = 0;
    char* spath[128];
    stokens = tokenize(src, spath);

    int dtokens = 0;
    char* dpath[128];
    dtokens = tokenize(dest, dpath);

    int dirIsReal = 0;

    char* addr = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED){
        printf("Error: Unable to map disk image\n");
        exit(1);
    }

    if(fileExists(src)){
        printf("File exists\n");
    }else{
        printf("File not found\n");
        exit(1);
    }

    if(dirExists(dpath, dtokens, addr)){
        printf("Directory exists\n");
        dirIsReal = 1;
    }else{
        printf("Directory not found\n");
        exit(1);
    }

    if(diskHasSpace(addr, buffer.st_size)){
        printf("Disk has space\n");
    }else{
        printf("Disk lacks space\n");
        exit(1);
    }

    struct dir_entry_t* destination = makeDirEntry(dpath, dtokens, addr);
    printf("Directory created %s\n", destination->filename);
    putfile(addr, src, destination);

    return 0;
}