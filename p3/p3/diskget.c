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
//yo
struct dir_entry_t* findFile(char* path[128], char* addr, int tokens){
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
                if(strcmp(working_dir->filename, path[i]) == 0){
                    return working_dir;
                }
                if(working_dir->status == 0x00){
                    printf("File not found\n");
                    exit(1);
                }
                if(i != tokens - 1){
                    addr = addr + working_dir->starting_block * superblock->block_size;
                }
            }
    return working_dir;
}

void copy_to_disk(struct dir_entry_t* file, char* addr, char* dest){
    FILE* destfile = fopen(dest, "wb");
    if(destfile == NULL){
        printf("Error: Unable to open destination file\n");
        exit(1);
    }
    char* buffer = (char*)malloc(file->size);
    int blocks = se32(file->size) / BLOCK_SIZE;
    if(se32(file->size) % BLOCK_SIZE != 0){
        blocks++;
    }
    for(i = 0; i < blocks; i++){
        memcpy(buffer + i * BLOCK_SIZE, addr + se32(file->starting_block) * BLOCK_SIZE + i * BLOCK_SIZE, BLOCK_SIZE);
    }
    fwrite(buffer, se32(file->size), 1, destfile);
    fclose(destfile);
    free(buffer);
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

    char* addr = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    struct dir_entry_t* file = findFile(spath, addr, stokens);
    if(strcmp(file->filename, spath[stokens - 1]) != 0){
        printf("Error: File not found\n");
        exit(1);
    }
    copy_to_disk(file, addr, dest);
    return 0;
}