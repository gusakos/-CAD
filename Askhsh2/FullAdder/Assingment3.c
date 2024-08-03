#include <stdlib.h>
#include <string.h>

#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#define GATE 1
#define SYBSYSTEM 2
#define GATES_COMP "COMPONENT LIBRARY"
#define FULL_ADDER_COMP "COMP FULL_ADDER"



        /*  Defining FULL_ADDER Comp*/
typedef struct {
    char* ID;
    int type;               // GATE OR SYBSYSTEM
    char* name;
    int num_inputs;
    int num_outputs;
    char** inputs;
    char** outputs;

}component;

typedef struct {
    char* ID;
    char* name;
    int num_inputs;
    int num_outputs;
    char** inputs;
    char** outputs;
    int num_comps;
    component** components;

}subsystem;



/*Opens the given file and gets the first line starting from the given offset until it fins a \n charater , 
it returns the ||# of chars readen counting also the  \n*/

int read_line_from_file(char **line, char *filename, size_t *len, int offset) {

    FILE *fp;
    int nread;

    // open the file
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(-1);
    }

    // seek to offset and read a line
    fseek(fp, offset, SEEK_SET);
    nread = getline(line, len, fp);

    fclose(fp);

    return nread;
}

int getSubSystemFromFile(char* file){
    if (file == NULL )
        return 1;

    char* line= NULL;
    int offset=0;
    size_t len= 0;
    int num_read=0;

    num_read= read_line_from_file(&line,file,&len,offset);

    char* substring= FULL_ADDER_COMP;

    char* ptr= strstr(line,substring);

   if (ptr != NULL) {
    printf("Component Library found: %s\n", ptr);
    return(0);
} else {
    printf("Component Library not found.\n");
    exit(-1);
}

    
}


int main(){

int a;
a=getSubSystemFromFile("Comp_full_adder.txt");

}


