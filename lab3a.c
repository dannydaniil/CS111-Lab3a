/*
TODO: change date format ==> use piazza post
      test using debugfs
      pass the sanity check
*/
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
#include <time.h>

#include "ext2_fs.h"

//global variables
char* fs_name;
int fs_fd;
struct ext2_super_block super;
struct ext2_group_desc group;
struct ext2_dir_entry dir;
int curr_offset, base_offset;
uint32_t block_size;

int num_directories = 0;
int num_inodes = 0;
int* directories,* dir_inodes,* inodes,* inodes_offset;
int* inode_map;

#define SUPERBLOCK_OFFSET       1024
#define OFFSET_ADDEND           40

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

     block_size = (EXT2_MIN_BLOCK_SIZE << super.s_log_block_size);
    printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
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
     printf("GROUP,0,%d,%d,%d,%d,%d,%d,%d\n",
        super.s_blocks_count,super.s_inodes_per_group,
        group.bg_free_blocks_count, group.bg_free_inodes_count,
        group.bg_block_bitmap, group.bg_inode_bitmap, group.bg_inode_table
    );
}

void analyzeBitmap(){

    int status;
    unsigned char entry;
    uint32_t start = group.bg_block_bitmap * block_size;

    int i, j;
    for(i = 0; i < block_size ; i++){
        status = pread(fs_fd, &entry, 1, start + i);
        if( status  ==  -1 ){
             print_error_message(errno,2);
         }

         unsigned char mask = 0x1;
         for(j = 0; j < 8 ; j++){

             if ( (entry & mask ) == 0 ){
                 printf("BFREE,%d\n", (8 * i) + j + 1 );
             }
             mask = mask << 1;
         }
    }

    //place start at inode bitmap
    start = group.bg_inode_bitmap * block_size;

    inode_map = (int*) malloc(8 * block_size * sizeof(int));

    for(i = 0; i < block_size ; i++ ){
        status = pread(fs_fd, &entry, 1, start + i);
        if( status  ==  -1 ){
             print_error_message(errno,2);
         }

         unsigned char mask = 0x1;
         for(j = 0; j < 8 ; j++){
             if ( (entry & mask ) == 0 ){
                 printf("IFREE,%d\n", (8 * i) + j + 1 );
                 inode_map[(8 * i) + j + 1] = 0;
             } else {
                 inode_map[(8 * i) + j + 1] = 1;
             }
             mask = mask << 1;
         }
    }
//end of analyzeBitmap
}

void analyzeInodes(){

    uint32_t start;
    uint32_t end;
    struct ext2_inode inode;
    int status;
    char file_type[2];

    //arrays to be used in analyzeDirectories
    directories = (int*)malloc(super.s_inodes_count * sizeof(int));
    dir_inodes = (int*)malloc(super.s_inodes_count * sizeof(int));
    inodes = (int*)malloc(super.s_inodes_count * sizeof(int));
    inodes_offset = (int*)malloc(super.s_inodes_count * sizeof(int));

    start = group.bg_inode_table * block_size;
    end = start + (super.s_inodes_count * sizeof(struct ext2_inode));


    int i, j;
    for( i = start; i < end ; i += sizeof(struct ext2_inode)){

        if(inode_map[i] == 1){

            status = pread( fs_fd, &inode,sizeof(struct ext2_inode), i );
            if( status  ==  -1 ){
                 print_error_message(errno,2);
             }
            if( (inode.i_links_count != 0) && (inode.i_mode != 0) ){

                inodes_offset[num_inodes] = i;
                inodes[num_inodes] = i + 1;
                num_inodes++;
                if(inode.i_mode & 0x8000){
                    strcpy(file_type,"f");
                }else if (inode.i_mode & 0x4000){
                    directories[num_directories] = i;
                    dir_inodes[num_directories] = i + 1;
                    num_directories ++;
                    strcpy(file_type,"d");
                } else if (inode.i_mode & 0xA000){
                    strcpy(file_type,"s");
                } else{
                    strcpy(file_type,"?");
                }

                char aBuff[30];
                char cBuff[30];
                char mBuff[30];

                time_t a_time = inode.i_atime;
                time_t c_time = inode.i_ctime;
                time_t m_time = inode.i_mtime;

                struct tm* a_struct = localtime(&a_time);
                struct tm* c_struct = localtime(&c_time);
                struct tm* m_struct = localtime(&m_time);

                strftime(aBuff,30,"%m/%d/%g %H:%M:%S",a_struct);
                strftime(cBuff,30,"%m/%d/%g %H:%M:%S",c_struct);
                strftime(mBuff,30,"%m/%d/%g %H:%M:%S",m_struct);


                printf("INODE,%d,%s,%d,%d,%d,%d,%s,%s,%s,%d,%d",
                i +1, file_type, inode.i_mode, inode.i_uid, inode.i_gid, inode.i_links_count,
                cBuff, mBuff, aBuff, inode.i_size,inode.i_blocks
                );

                for(j = 0; j < 15 ; j++){
                    if(inode.i_block[j]){
                        printf(",%d",inode.i_block[j]);
                    }else{
                        printf(",0");
                    }
                }
                printf("\n");

            }
        }
    }
//end of analyzeInodes
}

void generateDirectoryMessage(int parent_num, int end_limit) {
    while (curr_offset < end_limit) {
        if (pread(fs_fd, &dir.inode, 4, curr_offset) == -1) { print_error_message(errno, 2); }
        if (pread(fs_fd, &dir.rec_len, 2, curr_offset + 4) == -1) { print_error_message(errno, 2); }
        if (pread(fs_fd, &dir.name_len, 1, curr_offset + 6) == -1) { print_error_message(errno, 2); }
        if (pread(fs_fd, &dir.name, dir.name_len, curr_offset + 8) == -1) { print_error_message(errno, 2); }
        if (dir.name_len < EXT2_NAME_LEN) {
            dir.name[dir.name_len] = '\0';
        } else {
            dir.name[EXT2_NAME_LEN - 1] = '\0';
        }
        if (dir.inode == 0) {
            curr_offset += dir.rec_len;
        } else {
            const char * dirent = "DIRENT";
            printf("%s,%d,%d,%d,%d,%d,\'%s\'\n", dirent, dir_inodes[parent_num], curr_offset - base_offset,
                    dir.inode, dir.rec_len, dir.name_len, dir.name);
            curr_offset += dir.rec_len;
        }
    }
}

void analyzeDirectory() {
    uint32_t super_bsize = EXT2_MIN_BLOCK_SIZE << super.s_log_block_size;
    int i, j, k;
    for (i = 0; i < num_directories; i++) {
        //direct blocks
        uint32_t offset;
        for (j = 0; j < EXT2_NDIR_BLOCKS; j++) {
            if (pread(fs_fd, &offset, 4, (directories[i] + OFFSET_ADDEND + (j * 4))) == -1) { print_error_message(errno, 2); }
            if (offset == 0) { continue; }
            curr_offset = super_bsize * offset;
            base_offset = curr_offset;
            generateDirectoryMessage(i, curr_offset + super_bsize);
        }

        //indirect blocks
        if (pread(fs_fd, &offset, 4, (directories[i] + OFFSET_ADDEND + (EXT2_IND_BLOCK * 4))) == -1) { print_error_message(errno, 2); }
        if (offset == 0) { continue; }
        for (j = 0; j < super_bsize / 4; j++) {
            //depends on how inodes are organized
            curr_offset = super_bsize * offset + (j * 4);
            uint32_t block;
            if (pread(fs_fd, &block, 4, curr_offset) == -1) { print_error_message(errno, 2); }
            if (block != 0) {
                curr_offset = block * super_bsize;
                base_offset = curr_offset;
                generateDirectoryMessage(i, curr_offset + super_bsize);
            }
        }
        //double indirect blocks
        if (pread(fs_fd, &offset, 4, directories[i] + OFFSET_ADDEND + (EXT2_DIND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
        if (offset == 0) { continue; }
        for (j = 0; j < super_bsize / 4; j ++) {
            //find how inodes are organized
            curr_offset = super_bsize * offset + (j * 4);
            uint32_t block;
            if (pread(fs_fd, &block, 4, curr_offset) == -1) { print_error_message(errno, 2); }
            if (block == 0) { continue; }
            int k;
            for (k = 0; k < super_bsize / 4; k++) {
                uint32_t block2;
                if (pread(fs_fd, &block2, 4, block * super_bsize + (k * 4)) == -1) { print_error_message(errno, 2); }
                if (block2 == 0) { continue; }
                curr_offset = block2 * super_bsize;
                base_offset = curr_offset;
                generateDirectoryMessage(i, curr_offset + super_bsize);
            }
        }

        //triple indirect blocks
        if (pread(fs_fd, &offset, 4, directories[i] + OFFSET_ADDEND + (EXT2_TIND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
        if (offset == 0) { continue; }
        for (j = 0; j < super_bsize / 4; j++) {
            curr_offset = super_bsize * offset + (j * 4);
            uint32_t block;
            if (pread(fs_fd, &block, 4, curr_offset == -1)) { print_error_message(errno, 2); }
            if (block == 0) { continue; }
            for (k = 0; k < super_bsize / 4; k++) {
                uint32_t block2;
                if (pread(fs_fd, &block2, 4, block * super_bsize + (k * 4)) == -1) { print_error_message(errno, 2); }
                if (block2 == 0) { continue; }
                int l;
                for (l = 0; l < super_bsize / 4; l++) {
                    uint32_t block3;
                    if (pread(fs_fd, &block3, 4, block2 * super_bsize + (l * 4)) == -1) { print_error_message(errno, 2); }
                    if (block3 == 0) { continue; }
                    curr_offset = block3 * super_bsize;
                    base_offset = curr_offset;
                    generateDirectoryMessage(i, curr_offset + super_bsize);
                }
            }
        }

    }
}

void generateIndirectMessage(int inode_num, int indirection_level, int offset, int base_offset,
        int indirect_block, int block) {
    const char* indirect = "INDIRECT";
    int indirection_offset;
    switch (indirection_level) {
        case 1:
            indirection_offset = EXT2_IND_BLOCK;
            break;
        case 2:
            indirection_offset = EXT2_DIND_BLOCK;
            break;
        case 3:
            indirection_offset = EXT2_TIND_BLOCK;
            break;
    }
    printf("%s,%d,%d,%d,%d,%d\n", indirect, inodes[inode_num], indirection_level,
            indirection_offset + (offset - base_offset) / 4, indirect_block, block);

}

void analyzeIndirect() {
    uint32_t super_bsize = EXT2_MIN_BLOCK_SIZE << super.s_log_block_size;
    int i, j, k, l;
    //TODO: get inode_count
    for (i = 0; i < num_inodes; i++) {
        uint32_t block;

        //single indirect
        //TODO: populate inodes_offset array
        if (pread(fs_fd, &block, 4, inodes_offset[i] + OFFSET_ADDEND + (EXT2_IND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
        int offset = block * super_bsize;
        int base_offset = offset;
        for (j = 0; j < super_bsize; j++) {
            uint32_t block2;
            if (pread(fs_fd, &block2, 4, offset) == -1) { print_error_message(errno, 2); }
            if (block2 == 0) { continue; }
            generateIndirectMessage(i, 1, offset, base_offset, block, block2);
            offset += 4;
        }

        //double indirect
        if (pread(fs_fd, &block, 4, inodes_offset[i] + OFFSET_ADDEND + (EXT2_DIND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
        offset = block * super_bsize;
        base_offset = offset;
        for (j = 0; j < super_bsize / 4; j++) {
            uint32_t block2;
            if (pread(fs_fd, &block2, 4, offset) == -1) { print_error_message(errno, 2); }
            if (block2 == 0) { continue; }
            generateIndirectMessage(i, 2, offset, base_offset, block, block2);
            offset += 4;
            int offset2 = block2 * super_bsize;
            int base_offset2 = offset2;
            for (k = 0; k < super_bsize / 4; k++) {
                uint32_t block3;
                if (pread(fs_fd, &block3, 4, offset2) == -1) { print_error_message(errno, 2); }
                if (block3 == 0) { continue; }
                base_offset = offset2;
                generateIndirectMessage(i, 2, offset2, base_offset2, block2, block3);
                offset2 += 4;
            }
        }

        //triple indirect
        if (pread(fs_fd, &block, 4, inodes_offset[i] + OFFSET_ADDEND + (EXT2_TIND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
        offset = block * super_bsize;
        base_offset = offset;
        for (j = 0; j < super_bsize; j++) {
            uint32_t block2;
            if (pread(fs_fd, &block2, 4, offset) == -1) { print_error_message(errno, 2); }
            if (block2 == 0) { continue; }
            generateIndirectMessage(i, 3, offset, base_offset, block, block2);
            offset += 4;
            int offset2 = block2 * super_bsize;
            int base_offset2 = offset2;
            for (k = 0; k < super_bsize / 4; k++) {
                uint32_t block3;
                if (pread(fs_fd, &block3, 4, offset2) == -1) { print_error_message(errno, 2); }
                if (block3 == 0) { continue; }
                generateIndirectMessage(i, 3, offset2, base_offset2, block2, block3);
                offset2 += 4;
                int offset3 = block3 * super_bsize;
                int base_offset3 = offset3;
                for (l = 0; l < super_bsize / 4; l++) {
                    uint32_t block4;
                    if (pread(fs_fd, &block4, 4, offset3) == -1) { print_error_message(errno, 2); }
                    if (block4 == 0) { continue; }
                    generateIndirectMessage(i, 3, offset3, base_offset3, block3, block4);
                    offset += 4;
                }
            }
        }
    }
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

analyzeSuper();
analyzeGroup();
analyzeBitmap();
analyzeInodes();
analyzeDirectory();
analyzeIndirect();

free(directories);
free(dir_inodes);
free(inodes);
free(inodes_offset);

close(fs_fd);
//end of main
exit (0);
}
