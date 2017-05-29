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
uint32_t block_size;

#define SUPERBLOCK_OFFSET       1024

void print_usage(){
    printf("Usage: lab3a file_system_name\n");
}

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

     block_size = (1024 << super.s_log_block_size);

    fprintf(report_fd, "SUPERBLOCK, %d, %d, %d, %d, %d, %d, %d\n",
        super.s_blocks_count,super.s_inodes_count, block_size,
        super.s_inode_size, super.s_blocks_per_group, super.s_inodes_per_group,
        super.s_first_ino
    );
}

void analyzeGroup(){
    int status;
    status = pread(fs_fd,&group, sizeof(struct ext2_group_desc), SUPERBLOCK_OFFSET + sizeof(struct ext2_super_block));
    if( status  ==  -1 ){
         print_error_message(errno,2);
     }

     //always 1 group
     fprintf(report_fd, "GROUP, 0, %d, %d, %d, %d, %d, %d, %d\n",
        super.s_blocks_count,super.s_inodes_per_group,
        group.bg_free_blocks_count, group.bg_free_inodes_count,
        group.bg_block_bitmap, group.bg_inode_bitmap, group.bg_inode_table
    );
}

void analyzeBitmap(){

    unsigned char entry;
    int status;
    uint32_t start = group.bg_block_bitmap * block_size;

    int i;
    int j;
    for(i = 0; i < block_size ; i++){
        status = pread(fs_fd, &entry, 1, start + i);
        if( status  ==  -1 ){
             print_error_message(errno,2);
         }

         unsigned char mask = 0x1;
         for(j = 0; j < 8 ; j++){

             if ( (entry & mask ) == 0 ){
                 fprintf(report_fd, "BFREE %d\n", (8 * i) + j + 1 );
             }
             mask = mask << 1;
         }
    }

    //place start at inode bitmap
    start = group.bg_inode_bitmap * block_size;

    for(i = 0; i < block_size ; i++ ){
        status = pread(fs_fd, &entry, 1, start + i);
        if( status  ==  -1 ){
             print_error_message(errno,2);
         }

         unsigned char mask = 0x1;
         for(j = 0; j < 8 ; j++){
             if ( (entry & mask ) == 0 ){
                 fprintf(report_fd, "IFREE %d\n", (8 * i) + j + 1 );
             }
             mask = mask << 1;
         }
    }
//end of analyzeBitmap
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
analyzeBitmap();
analyzeInodes();

close(fs_fd);
fclose(report_fd);
//end of main
}
