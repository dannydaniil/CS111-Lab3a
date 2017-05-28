#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include "ext2_fs.h"

//global variables
char* fs_name;
int fs_fd;
FILE* report_fd;
struct ext2_super_block super;
struct ext2_group_desc group;

#define SUPERBLOCK_OFFSET       1024


void print_error_message(int err_num, int exit_code) {
    fprintf(stderr, "%s\n", strerror(err_num));
    exit(exit_code);
}


void analyzeSuper(){
    //pread(fd, buff, count , offset);
    int status;
    status = pread(fs_fd,&super, sizeof(struct ext2_super_block), SUPERBLOCK_OFFSET);
    if( status  ==  -1 ){
         print_error_message(errno,2);
     }

    fprintf(report_fd, "SUPERBLOCK, %d, %d, %d, %d, %d, %d, %d ",
        super.s_blocks_count,super.s_inodes_count, 1024 << super.s_log_block_size,
        super.s_inode_size, super.s_blocks_per_group, super.s_inodes_per_group,
        super.s_first_ino
        );
}

void analyzeGroup(){
    int status;
    status = pread(fs_fd,&my_group_desc, sizeof(struct ext2_group_desc), SUPERBLOCK_OFFSET);
    if( status  ==  -1 ){
         print_error_message(errno,2);
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
        fs_name = argv[1];
        if(fs_name == NULL){
            print_error_message(errno,2);
        }
    }

    fs_fd = open(fs_name, O_RDONLY);
    if( fs_fd == -1 ){
        print_error_message(errno,2);
    }

report_fd = fopen("report.csv","a");

analyzeSuper();
analyzeGroup();

//end of main
}
