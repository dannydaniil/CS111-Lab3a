#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BLOCK_COUNT 12

//global variables
char* fs_name;
int fs_fd, directory_fd, indirect_fd;
struct super_t * super;

uint8_t buffer_8;
uint16_t buffer_16;
uint32_t buffer_32;

void print_error_message(int err_num, int exit_code) {
    fprintf(stderr, "%s\n", strerror(err_num));
    exit(exit_code);
}

void analyzeSuper(){
    

}

void generateDirectoryMessage(int curr_entry) {
    //TODO: replace with body of analyzeDirectory();
}

void analyzeDirectory() {
    directory_fd = open("directory.csv", OWRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    int i, j, k;
    int curr_entry;
    //TODO: get directory_count
    for (i = 0; i < directory_count; i++) {
        curr_entry = 0;
        for (j = 0; j < BLOCK_COUNT; j++) {
            uint32_t offset;
            //TODO" need node
            if (pread(fs_fd, offset, 4,... ) == -1) { print_error_message(errno, 2); }
            if (offset == 0) { continue; }
            int curr_offset = super->blockSize * offset;
            for (k = 0; k < super->blockSize; i++) {
                uint8_t name_length;
                if (pread(fs_fd, &name_length, 1, curr_offset + 6) == -1) { print_error_message(errno, 2); }

                uint16_t entry_length;
                if (pread(fs_fd, &entry_length, 2, curr_offset + 4) == -1) { print_error_message(errno, 2); }

                uint32_t inode;
                if (pread(fs_fd, &inode, 4, curr_offset) == -1) { print_error_message(errno, 2); }

                if (inode == 0) {
                    curr_offset += entry_length;
                    curr_entry++;
                } else {
                    //TODO:replace inode_parent with whatever daniel uses 
                    char name_char;
                    dprintf(directory_fd, "%d,%d,%d,%d,%d,\"", inode_parent, curr_entry, entry_length, name_length, inode);
                    curr_entry++;
                    int l;
                    for (l = 0; l < name_length; l++) {
                        if (pread(fs_fd, &name_char, 1, curr_offset + 8 + l) == -1) { print_error_message(errno, 2); }
                        dprintf(directory_fd, "%c", name_char);
                    }
                    dprintf(directory_fd, "\"\n");
                    curr_offset += entry_length;
                }
            }
        }
    }
}

void analyzeIndirect() {
    indirect_fd = creat("indirect.csv", S_IRWXU);
    int i, j;
    int curr_entry;
    //TODO: get inode_count
    for (i = 0; i < inode_count; i++) {
        curr_entry = 0;
        uint32_t curr_block;
    }
}

void print_usage(){
    printf("Usage: lab3a file_system_name\n");
}

int main(int argc, char* argv[]){

    if(argc != 2){
        fprintf(stderr, "Wrong number of arguments provided.\n");
        print_usage();
        exit(1);
    } else{
        fs_name = malloc(sizeof(char) * strlen(argv[1]+1));
        if(fs_name == NULL){
            print_error_message(errno, 2);
        }
    }

    fs_fd = open(fs_name, O_RDONLY);
    if( fs_fd == -1 ){
            print_error_message(errno, 2);
    }





analyzeSuper();
analyzeDirectory();
analyzeIndirect();
//end of main
}
