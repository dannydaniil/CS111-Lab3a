#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//global variables
char* fs_name;
int fs_fd;

void analyzeSuper(){
    

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
            fprintf(stderr, "Error allocating memory: %s\n",strerror(errno));
            exit(2);
        }
    }

    fs_fd = open(fs_name, O_RDONLY);
    if( fs_fd == -1 ){
        fprintf(stderr, "Error while opening file%s\n",strerror(errno));
        exit(2);
    }





analyzeSuper();


//end of main
}
