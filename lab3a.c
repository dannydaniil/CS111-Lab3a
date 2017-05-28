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

void print_error_message(int err_num, int exit_code) {
    fprintf(stderr, "%s\n", strerror(err_num));
    exit(exit_code);
}

void analyzeSuper(){
    

}

void analyzeDirectory() {
    directory_fd = creat("directory.csv", S_IRWXU);
    int i, j;
    int curr_entry;
    //TODO: get directory_count
    for (i = 0; i < directory_count; i++) {
        curr_entry = 0;
        for (j = 0; j < BLOCK_COUNT; j++) {
            uint32_t offset;
            //TODO" need code
            if (pread(fs_fd, offset, 4, 
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
