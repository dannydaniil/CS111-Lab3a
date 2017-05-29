#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "ext2_fs.h"

#define SUPER_BLOCK_OFFSET 1024
#define SUPER_BLOCK_SIZE 1024
#define INODE_BLOCK_POINTER_WIDTH 4
#define GROUP_DESCRIPTOR_TABLE_SIZE 32
#define INODE_SIZE 128

char *fileName;
int fs_fd, superFd, groupFd, bitmapFd, inodeFd, directoryFd, indirectFd;
int status, sBuf32;
int numGroups;
uint64_t buf64;
uint32_t buf32;
uint16_t buf16;
uint8_t buf8;
struct super_t *super;
struct group_t *group;
int **validInodeDirectories;
int validInodeDirectoryCount = 0;
int *validInodes;
int validInodeCount = 0;

struct super_t {
	uint16_t magicNumber;
	uint32_t inodeCount, blockCount, blockSize, fragmentSize, blocksPerGroup, inodesPerGroup, fragmentsPerGroup, firstDataBlock;
};

struct group_t {
	uint16_t containedBlockCount, freeBlockCount, freeInodeCount, directoryCount;
	uint32_t inodeBitmapBlock, blockBitmapBlock, inodeTableBlock;
};

void parseArgs(int argc, char **argv){
	if(argc != 2){
		fprintf(stderr, "Error: Please provide a disk image name as the only argument\n");
		exit(EXIT_FAILURE);
	}
	else{
		fileName = malloc( (strlen(argv[1]) + 1) * sizeof(char));
		fileName = argv[1];
		return;
	}
}

void initVars(){
	fs_fd = open(fileName, O_RDONLY);
	super = malloc(sizeof(struct super_t));
	group = malloc(sizeof(struct group_t));
}

void parseSuper(){

	// Create csv file
	superFd = creat("super.csv", S_IRWXU);

	// Magic number
	pread(fs_fd, &buf16, 2, SUPER_BLOCK_OFFSET + 56);
	super->magicNumber = buf16;
	dprintf(superFd, "%x,", super->magicNumber);

	// inode count
	pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + 0);
	super->inodeCount = buf32;
	dprintf(superFd, "%d,", super->inodeCount);

	// Number of blocks
	pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + 4);
	super->blockCount = buf32;
	dprintf(superFd, "%d,", super->blockCount);

	// Block size
	pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + 24);
	super->blockSize = 1024 << buf32;
	dprintf(superFd, "%d,", super->blockSize);	

	// Fragment size
	pread(fs_fd, &sBuf32, 4, SUPER_BLOCK_OFFSET + 28);
	if(sBuf32 > 0){
		super->fragmentSize = 1024 << sBuf32;
	}
	else{
		super->fragmentSize = 1024 >> -sBuf32;
	}
	dprintf(superFd, "%d,", super->fragmentSize);

	// Blocks per group
	pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + 32);
	super->blocksPerGroup = buf32;
	dprintf(superFd, "%d,", super->blocksPerGroup);	

	// inodes per group
	pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + 40);
	super->inodesPerGroup = buf32;
	dprintf(superFd, "%d,", super->inodesPerGroup);	

	// Fragments per group
	pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + 36);
	super->fragmentsPerGroup = buf32;
	dprintf(superFd, "%d,", super->fragmentsPerGroup);

	// First data block
	pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + 20);
	super->firstDataBlock = buf32;
	dprintf(superFd, "%d\n", super->firstDataBlock);

	// Close csv file
	close(superFd);
}

void parseGroup(){

	// Create csv file
	groupFd = creat("group.csv", S_IRWXU);

	// Calculate number of groups
	numGroups = ceil((double)super->blockCount / super->blocksPerGroup);
	double remainder = numGroups - ((double)super->blockCount / super->blocksPerGroup);

	group = malloc(numGroups * sizeof(struct group_t));

	int i;
	for(i = 0; i < numGroups; i++){

		// Number of contained blocks
		if(i != numGroups - 1 || remainder == 0){
			group[i].containedBlockCount = super->blocksPerGroup;
			dprintf(groupFd, "%d,", group[i].containedBlockCount);
		}
		else{
			group[i].containedBlockCount = super->blocksPerGroup * remainder;
			dprintf(groupFd, "%d,", group[i].containedBlockCount);
		}

		// Number of free blocks
		pread(fs_fd, &buf16, 2, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE_SIZE) + 12);
		group[i].freeBlockCount = buf16;
		dprintf(groupFd, "%d,", group[i].freeBlockCount);

		// Number of free inodes
		pread(fs_fd, &buf16, 2, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE_SIZE) + 14);
		group[i].freeInodeCount = buf16;
		dprintf(groupFd, "%d,", group[i].freeInodeCount);

		// Number of directories
		pread(fs_fd, &buf16, 2, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE_SIZE) + 16);
		group[i].directoryCount = buf16;
		dprintf(groupFd, "%d,", group[i].directoryCount);

		// Free inode bitmap block
		pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE_SIZE) + 4);
		group[i].inodeBitmapBlock = buf32;
		dprintf(groupFd, "%x,", group[i].inodeBitmapBlock);

		// Free block bitmap block
		pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE_SIZE) + 0);
		group[i].blockBitmapBlock = buf32;
		dprintf(groupFd, "%x,", group[i].blockBitmapBlock);

		// Inode table start block
		pread(fs_fd, &buf32, 4, SUPER_BLOCK_OFFSET + SUPER_BLOCK_SIZE + (i * GROUP_DESCRIPTOR_TABLE_SIZE) + 8);
		group[i].inodeTableBlock = buf32;
		dprintf(groupFd, "%x\n", group[i].inodeTableBlock);
	}

	// Close csv file
	close(groupFd);
}

void parseBitmap(){

	// Create csv file
	bitmapFd = creat("bitmap.csv", S_IRWXU);

	// Check each group
	int i;
	for(i = 0; i < numGroups; i++){

		// Check each byte in block bitmap
		int j;
		for(j = 0; j < super->blockSize; j++){

			// Read a byte
			pread(fs_fd, &buf8, 1, (group[i].blockBitmapBlock * super->blockSize) + j);
			int8_t mask = 1;

			// Analyze the byte
			int k;
			for(k = 1; k <= 8; k++){
				int value = buf8 & mask;
				if(value == 0){
					dprintf(bitmapFd, "%x,", group[i].blockBitmapBlock);
					dprintf(bitmapFd, "%d", j * 8 + k + (i * super->blocksPerGroup));
					dprintf(bitmapFd, "\n");
				}
				mask = mask << 1;
			}
		}

		// Check each byte in inode bitmap
		for(j = 0; j < super->blockSize; j++){

			// Read a byte
			pread(fs_fd, &buf8, 1, (group[i].inodeBitmapBlock * super->blockSize) + j);
			int8_t mask = 1;

			// Analyze the byte
			int k;
			for(k = 1; k <= 8; k++){
				int value = buf8 & mask;
				if(value == 0){
					dprintf(bitmapFd, "%x,", group[i].inodeBitmapBlock);
					dprintf(bitmapFd, "%d", j * 8 + k + (i * super->inodesPerGroup));
					dprintf(bitmapFd, "\n");
				}
				mask = mask << 1;
			}
		}
	}

	// Close csv file
	close(bitmapFd);
}

void parseInode(){

	// Create csv file
	inodeFd = creat("inode.csv", S_IRWXU);

	// Allocate space to save valid directory inode information
	validInodeDirectories = malloc(super->inodeCount * sizeof(int*));
	int i;
	for(i = 0; i < super->inodeCount; i++){
		validInodeDirectories[i] = malloc(2 * sizeof(int));
	} 

	// Allocate space to save valid inode information
	validInodes = malloc(super->inodeCount * sizeof(int));

	// Check each group
	for(i = 0; i < numGroups; i++){

		// Check each byte in the inode bitmap
		int j;
		for(j = 0; j < super->blockSize; j++){

			// Read a byte
			pread(fs_fd, &buf8, 1, (group[i].inodeBitmapBlock * super->blockSize) + j);
			int8_t mask = 1;

			// Analyze the byte
			int k;
			for(k = 1; k <= 8; k++){
				int value = buf8 & mask;
				if(value != 0 && (j * 8 + k) <= super->inodesPerGroup){

					// Get inode number
					int inodeNumber = j * 8 + k + (i * super->inodesPerGroup);
					dprintf(inodeFd, "%d,", inodeNumber);

					// Get file type
					pread(fs_fd, &buf16, 2, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 0);
					validInodes[validInodeCount] = group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE;
					validInodeCount++;

					// Check for regular files
					if(buf16 & 0x8000){
						dprintf(inodeFd, "f,");
					}

					// Check for symbolic files
					else if(buf16 & 0xA000){
						dprintf(inodeFd, "s,");
					}

					// Check for directories
					else if(buf16 & 0x4000){
						validInodeDirectories[validInodeDirectoryCount][0] = group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE;
						validInodeDirectories[validInodeDirectoryCount][1] = inodeNumber;
						validInodeDirectoryCount++;
						dprintf(inodeFd, "d,");
					}

					// Check for other files
					else{
						dprintf(inodeFd, "?,");
					}

					// Get mode
					dprintf(inodeFd, "%o,", buf16);

					// Get owner
					uint32_t uid;
					pread(fs_fd, &buf32, 2, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 2);
					uid = buf32;
					pread(fs_fd, &buf32, 2, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 116 + 4);
					uid = uid | (buf32 << 16);
					dprintf(inodeFd, "%d,", uid);

					// Get group
					uint32_t gid;
					pread(fs_fd, &buf32, 2, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 24);
					gid = buf32;
					pread(fs_fd, &buf32, 2, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 116 + 6);
					gid = gid | (buf32 << 16);
					dprintf(inodeFd, "%d,", gid);

					// Get link count
					pread(fs_fd, &buf16, 2, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 26);
					dprintf(inodeFd, "%d,", buf16);

					// Get creation time
					pread(fs_fd, &buf32, 4, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 12);
					dprintf(inodeFd, "%x,", buf32);

					// Get modification time
					pread(fs_fd, &buf32, 4, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 16);
					dprintf(inodeFd, "%x,", buf32);

					// Get access time
					pread(fs_fd, &buf32, 4, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 8);
					dprintf(inodeFd, "%x,", buf32);

					// Get file size
					uint64_t size;
					pread(fs_fd, &buf64, 4, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 4);
					size = buf64;					
					pread(fs_fd, &buf64, 4, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 108);
					size = size | (buf64 << 32);
					dprintf(inodeFd, "%ld,", size);

					// Get number of blocks
					pread(fs_fd, &buf32, 4, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 28);
					dprintf(inodeFd, "%d,", buf32/(super->blockSize/512));

					// Get block pointers
					int x;
					for(x = 0; x < 15; x++){
						pread(fs_fd, &buf32, 4, group[i].inodeTableBlock * super->blockSize + (j * 8 + k - 1) * INODE_SIZE + 40 + (x * 4));
						if(x != 14){
							dprintf(inodeFd, "%x,", buf32);
						}
						else{
							dprintf(inodeFd, "%x", buf32);
						}
					}

					dprintf(inodeFd, "\n");

					buf16 = 0;
					buf32 = 0;
					buf64 = 0;
				}
				mask = mask << 1;
			}
		}
	}

	// Close csv file
	close(inodeFd);
}

void print_error_message(int err_num, int exit_code) {
    fprintf(stderr, "%s\n", strerror(err_num));
    exit(exit_code);
}

void generateDirectoryMessage(int parent_num, int end_limit) {
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
            fprintf(stdout, "%s,%d,%d,%d,%d,%d,\'%s\'\n", dirent, dir_inode[parent_num], curr_offset, dir.inode, dir.rec_len, dir.name_len, dir.name);
            curr_offset += dir.rec_len;
        }
    }
}

void analyzeDirectory() {
    __u32 super_bsize = EXT2_MIN_BLOCK_SIZE << super.s_log_block_size;
    int i, j, k;
    //TODO: get directory_count
    for (i = 0; i < num_directories; i++) {
        //direct blocks
        __u32 offset;
        for (j = 0; j < EXT2_NDIR_BLOCKS; j++) {
            //TODO: check if offset is correct
            if (pread(fs_fd, &offset, 4, (directories[i] + 40 + (j * 4))) == -1) { print_error_message(errno, 2); }
            if (offset == 0) { continue; }
            curr_offset = super_bsize * offset;
            generateDirectoryMessage(i, super_bsize * offset + super_bsize); 
        }
        
        //indirect blocks
        //TODO: get pread arg
        if (pread(fs_fd, &offset, 4, (directories[i] + 40 + (EXT2_IND_BLOCK * 4))) == -1) { print_error_message(errno, 2); }
        if (offset == 0) { continue; }
        for (j = 0; j < super_bsize / 4; j++) {
            //depends on how inodes are organized
            curr_offset = super_bsize * offset + (j * 4);
            __u32 block;
            if (pread(fs_fd, &block, 4, curr_offset) == -1) { print_error_message(errno, 2); }
            if (block != 0) {
                curr_offset = block * super_bsize;
                generateDirectoryMessage(i, block * super_bsize + super_bsize);
            }
        }

        //double indirect blocks
        //TODO: find where the double indirect blocks are
        if (pread(fs_fd, &offset, 4, directories[i] + 40 + (EXT2_DIND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
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
                generateDirectoryMessage(i, block2 * super_bsize + super_bsize);
            }
        }

        //triple indirect blocks
        //TODO: find where triple indirect blocks are
        if (pread(fs_fd, &offset, 4, directories[i] + 40 + (EXT2_TIND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
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
                int l;
                for (l = 0; l < super_bsize / 4; l++) {
                    __u32 block3;
                    if (pread(fs_fd, &block3, 4, block2 * super_bsize + (l * 4)) == -1) { print_error_message(errno, 2); }
                    if (block3 == 0) { continue; }
                    curr_offset = block3 * super_bsize;
                    generateDirectoryMessage(i, block3 * super_bsize + super_bsize);
                }
            }
        }

    }
}

void generateIndirectMessage(int inode_num, int indirection_level, int offset, int indirect_block, int block) {
    const char* indirect = "INDIRECT";
    fprintf(stdout, "%s,%d,%d,%d,%d,%d\n" indirect, inodes[inode_num], indirection_level,
            offset, indirect_block, block);

}

void analyzeIndirect() {
    __u32 super_bsize = EXT2_MIN_BLOCK_SIZE << super.s_log_block_size;
    int i, j, k;
    //TODO: get inode_count
    for (i = 0; i < num_inodes; i++) {
        __u32 block;

        //single indirect
        //TODO: populate inodes_offset array
        if (pread(fs_fd, &block, inodes_offset[i] + 40 + (EXT2_IND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
        int offset = block * super_bsize;
        for (j = 0; j < super_bsize; j++) {
            __u32 block2;
            if (pread(fs_fd, &block2, 4, offset) == -1) { print_error_message(errno, 2); }
            if (block2 == 0) { continue; }
            generateIndirectMessage(i, 1, offset, block, block2);
            offset += 4;
        }

        //double indirect
        if (pread(fs_fd, &block, inodes_offset[i] + 40 + (EXT_DIND_BLOCK * 4)) == -1) { print_error_message(errno, 2); }
        offset = block * super_bsize;
        for (j = 0; j < super_bsize / 4; j++) {
            __u32 block2;
            if (pread(fs_fd, &block2, 4, offset) == -1) { print_error_message(errno, 2); }
            if (block2 == 0) { continue; }
            generateIndirectMessage(i, 2, offset, block, block2);
            offset += 4;
            offset2 = block2 * super_bsize;
            for (k = 0; k < super_bsize / 4; k++) {
                __u32 block3;
                if (pread(fs_fd, &block3, 4, offset2) == -1) { print_error_message(errno, 2); }
                if (block3 == 0) { continue; }
                generateIndirectMessage(i, 2, offset2, block2, block3);
                offset2 += 4;
            }
        }

        //

    }
}

int main(int argc, char **argv){
	parseArgs(argc, argv);
	initVars();
	parseSuper();
	parseGroup();
	parseBitmap();
	parseInode();
	analyzeDirectory();
	analyzeIndirect();
	close(fs_fd);
	exit(EXIT_SUCCESS);
}