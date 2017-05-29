#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ext2_fs.h"

//global variables
char* fs_name;
int fs_fd;
struct ext2_super_block * super;

int curr_offset;

int * directories, * dir_inodes;
int num_directories = 0;

struct ext2_super_block super;
struct ext2_dir_entry dir;


void print_error_message(int err_num, int exit_code) {
    fprintf(stderr, "%s\n", strerror(err_num));
    exit(exit_code);
}

void analyzeSuper(){
    

}

void analyzeInode() {
    directories = (int*)malloc(super.s_inodes_count * sizeof(int));
    dir_inodes = (int*)malloc(super.s_inodes_count * sizeof(int));
    __u32 super_bsize = EXT2_MIN_BLOCK_SIZE << super->s_log_block_size;
    int i;
    for (i = 0; i < super_bsize; i++) {
        
    }
}

void generateDirectoryMessage(int end_limit) {
    while (curr_offset < end_limit) {
        if (pread(fs_fd, &dir.inode, 4, curr_offset) == -1) { print_error_message(errno, 2); }
        if (pread(fs_fd, &dir.rec_len, 2, curr_offset + 4) == -1) { print_error_message(errno, 2); }
        if (pread(fs_fd, &dir.name_len, 1, curr_offset + 6) == -1) { print_error_message(errno, 2); }
        if (pread(fs_fd, &dir.name, dir.name_len, curr_offset + 8) == -1) { print_error_message(errno, 2); }

        if (dir.inode == 0) {
            curr_offset += dir.rec_len;
        } else {
    //TODO:replace inode_parent with whatever daniel uses 
            const char * dirent = "DIRENT";
            char name_char;
            fprintf(stdout, "%s,%d,%d,%d,%d,%d,\'%s\'\n", dirent, inode_parent, curr_offset, dir.inode, dir.rec_len, dir.name_len, dir.name);
            curr_offset += dir.rec_len;
        }
    }
}

void analyzeDirectory() {
    findDirectories();

    __u32 super_bsize = EXT2_MIN_BLOCK_SIZE << super->s_log_block_size;
    int i, j, k;
    //TODO: get directory_count
    for (i = 0; i < directory_count; i++) {
        //direct blocks
        __u32 offset;
        for (j = 0; j < EXT2_NDIR_BLOCKS; j++) {
            //TODO" need node
            if (pread(fs_fd, &offset, 4,... ) == -1) { print_error_message(errno, 2); }
            if (offset == 0) { continue; }
            curr_offset = super_bsize * offset;
            generateDirectoryMessage(super_bsize * offset + super_bsize); 
        }
        
        //indirect blocks
        //TODO: get pread arg
        if (pread(fs_fd, &offset, 4, ..) == -1) { print_error_message(errno, 2); }
        if (offset == 0) { continue; }
        for (j = 0; j < super_bsize / 4; j++) {
            //depends on how inodes are organized
            curr_offset = super_bsize * offset + (j * 4);
            __u32 block;
            if (pread(fs_fd, &block, 4, curr_offset) == -1) { print_error_message(errno, 2); }
            if (block != 0) {
                curr_offset = block * super_bsize;
                generateDirectoryMessage(block * super_bsize + super_bsize);
            }
        }

        //double indirect blocks
        //TODO: find where the double indirect blocks are
        if (pread(fs_fd, &offset, 4, ...) == -1) { print_error_message(errno, 2); }
        if (offset == 0) { continue; }
        for (j = 0; j < super_bsize / 4; j ++) {
            //find how inodes are organized
            curr_offset = super_bsize * offset + (j * 4);
            __u32 block;
            if (pread(fs_fd, &block, 4, curr_offset) == -1) { print_error_message(errno, 2); }
            if (block == 0) { continue; }
            int k;
            for (k = 0; k < super_bsize / 4; k++) {
                __u32 block2;
                if (pread(fs_fd, &block2, 4, block * super_bsize + (k * 4)) == -1) { print_error_message(errno, 2); }
                if (block2 == 0) { continue; }
                curr_offset = block2 * super_bsize;
                generateDirectoryMessage(block2 * super_bsize + super_bsize);
            }
        }

        //triple indirect blocks
        //TODO: find where triple indirect blocks are
        if (pread(fs_fd, &offset, 4, ...) == -1) { print_error_message(errno, 2); }
        if (offset == 0) { continue; }
        for (j = 0; j < super_bsize / 4; j++) {
            curr_offset = super_bsize * offset + (j * 4);
            __u32 block;
            if (pread(fs_fd, &block, 4, curr_offset == -1)) { print_error_message(errno, 2); }
            if (block == 0) { continue; }
            int k;
            for (k = 0; k < super_bsize / 4; k++) {
                __u32 block2;
                if (pread(fs_fd, &block2, 4, block * super_bsize + (k * 4)) == -1) { print_error_message(errno, 2); }
                if (block2 == 0) { continue; }

            }
        }

    }
}

void generateIndirectMessage() {
    const char* indirect = "INDIRECT";


}

void analyzeIndirect() {
    int i, j;
    //TODO: get inode_count
    for (i = 0; i < inode_count; i++) {
        __u32 curr_block;
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
