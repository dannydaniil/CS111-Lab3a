#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void print_usage(){
    printf("Usage: lab3a file_system_name\n");
}

int main(int argc, char* argv[]){

//name of file system to be analyzed
char* filename;

    if(argc != 2){
        fprintf(stderr, "Wrong number of arguments provided.\n");
        print_usage();
        exit(1);
    } else{
        filename = malloc(sizeof(char) * strlen(argv[1]+1));
        if(filename == NULL){
            fprintf(stderr, "Error allocating memory: %s\n",strerror(errno));
            exit(1);
        }
    }

//end of main
}
