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
int fs_fd, directory_fd, indirect_fd;
struct ext2_super_block * super;

int curr_entry, curr_offset;

void print_error_message(int err_num, int exit_code) {
    fprintf(stderr, "%s\n", strerror(err_num));
    exit(exit_code);
}

void analyzeSuper(){
    

}

void generateDirectoryMessage(int end_limit) {
    //TODO: replace with body of analyzeDirectory();
    __u8 name_length;
    __u16 entry_length;
    __u32 inode;
    while (curr_offset < end_limit) {
        if (pread(fs_fd, &name_length, 1, curr_offset + 6) == -1) { print_error_message(errno, 2); }
        if (pread(fs_fd, &entry_length, 2, curr_offset + 4) == -1) { print_error_message(errno, 2); }
        if (pread(fs_fd, &inode, 4, curr_offset) == -1) { print_error_message(errno, 2); }

        if (inode == 0) {
            curr_offset += entry_length;
            curr_entry;
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

void analyzeDirectory() {
    __u32 super_bsize = EXT2_MIN_BLOCK_SIZE << super->s_log_block_size;
    directory_fd = open("directory.csv", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    int i, j, k;
    //TODO: get directory_count
    for (i = 0; i < directory_count; i++) {
        //direct blocks
        curr_entry = 0;
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
                __u32 block2
                if (pread(fs_fd, &block2, 4, block * super_bsize + (k * 4)) == -1) { print_error_message(errno, 2); }
                if (block == 0) { continue; }
                curr_ofset = block2 * super_bsize;
                generateDirectoryMessage(block2 * super_bsize + super_bsize);
            }
        }

        //triple indirect blocks
        //TODO: find where triple indirect blocks are
        if (pread(fs_fd, &offset, 4, ...) == -1) { print_error_message(errno, 2); }
        if (offset == 0) { continue; }

    }
}

void analyzeIndirect() {
    indirect_fd = creat("indirect.csv", S_IRWXU);
    int i, j;
    int curr_entry;
    //TODO: get inode_count
    for (i = 0; i < inode_count; i++) {
        curr_entry = 0;
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
