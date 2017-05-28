#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ext2_fs.h"

//global variables
char* fs_name;
int fs_fd;


void print_error_message(int err_num, int exit_code) {
    fprintf(stderr, "%s\n", strerror(err_num));
    exit(exit_code);
}



void analyzeSuper(){
    //pread(fd, buff, count , offset);

    __u32 block_num;
    pread(fs_fd,&block_num, sizeof(__u32) ,1024 + sizeof(__u32));
    print_error_message(errno,2);





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
            print_error_message(errno,2);
        }
    }

    fs_fd = open(fs_name, O_RDONLY);
    if( fs_fd == -1 ){
        print_error_message(errno,2);
    }



analyzeSuper();


//end of main
}
