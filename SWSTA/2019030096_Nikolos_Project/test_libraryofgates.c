#include <stdlib.h>
#include <string.h>

#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#define GATE 1
#define SYBSYSTEM 2
#define GATES_COMP "COMPONENT LIBRARY"
#define FULL_ADDER_COMP "FULL_ADDER"
#define FULL_SUB_ADDER_COMP "FULL_SUBTRACTOR"
#define DELIM ";"
#define INS " IN:"
#define COMMA ","
#define OUTS " OUT:"
#define LIB "LIBRARY"

/**
 * A component can be a gate
 * 


*/
typedef struct {
    char* ID;
    int type;               
    char* name;
    int num_inputs;
    int num_outputs;
    char** inputs;
    char** outputs;
    

}component;

/* Every gate*/
typedef struct {
    char* name;
    int num_inputs;
    char** inputs;
    int  truthtable[4];
    int num_truthtable;
}gate;


typedef struct {
    int num_gates;
    gate** gates;
    char* file;
}library;






/*  Gets a pointer to a string and a delim to search, 
    if delim is found -> pointer now points to the substring A
    not containing the delim and every character before it
    Subsitrng B contaiinig every character before the delim is returned
    
    If delim is not found-> returns the whole string and pointer is set to NULL*/
char *split(char **str, char *delim) {

    char *begin, *end;

    begin = *str;
    if (begin == NULL) return NULL;

    if((end = strstr(*str, delim))){
        /* if delim is found, insert a null byte there and advance str */
        *end = '\0';
        *str = end+strlen(delim);
    } else {
        /* if this is the last token set str to NULL */
        *str = NULL;
    }

    return begin;
}

/* Gets  a string which we need to split into arguments using a delimeter and puts every argument in a list
List is passed as a char**  aleady made by the caller  , memory is allocated for every input in the list
and the list too
Returns the number of strings writen in the list
We need to free every input of char**l and l itself  */
int str_to_list(char *str, char ***l, char *delim) {

    char *cur;
    int i=0;

    while(str != NULL) {

        // get the list item
        cur = split(&str, delim);
        
        // allocate memory for the list entry
        (*l) = realloc((*l), sizeof(char*)*(i+1));
        if ((*l) == NULL) {
            fprintf(stderr, "malloc() error! not enough memory!\n");
            exit(-1);
        }

        // allocate memory for the item (plus a null byte)
        (*l)[i] = malloc(strlen(cur)+5);
        if ((*l)[i] == NULL) {
            fprintf(stderr, "malloc() error! not enough memory!\n");
            exit(-1);
        }

        // copy the item into the table
        strncpy((*l)[i], cur, strlen(cur)+1);  

        i++;
    }
    return i;
}

/*Opens the given file and gets the first line starting from the given offset until it finds a \n charater , 
it returns the # of chars readen counting also the  \n 
the given pointer **line now points to the string acquired 
even if line is null getline() allocates the need space for it*/
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



/* gets a file containing a library of gates and stores it
   into a library struct made of gates
   Freeing of library and each gate needs to be done by the caller*/
int getLibraryFromFile(char* file, library* lib){
    if (file == NULL )
        return 1;
    
    // new line will be stored here even if its set to null when getline is called
    char* line= NULL;

    // kepping track of the line we are currently in the file
    int offset=0;

    // needed for getline()
    size_t len= 0;

    // # of characters readen in new line to update the offset 
    int num_read=0;

    

    // get file's name to lib
    lib->file=strdup(file);

    // read first line
    num_read= read_line_from_file(&line,file,&len,offset);
    if(num_read==-1){
        printf("unable to read line ");
        exit(0);
    }
    // update offset
    offset+= num_read;


    // check if it's indeed the library file
    // strstr return pointer if found or null if not
    char* substring= LIB;

    char* ptr= strstr(line,substring);

    if(ptr== NULL){
        printf("File is not a library of gates");
        exit(0);
    }

   
    // set lib->gates to NULL so that realloc can resize it 
    lib->gates= NULL;

    //used in while
    int i=0;

    //read each gate by reading every line until the end of the file
    while(num_read!=-1){

        // move to next gate
        num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
            break;
        }
        // update offset
        offset+= num_read;
     
        // resize the space needed for gates  for every gate that is readen using realloc
        lib->gates= realloc(lib->gates,sizeof(gate*)*(i+1));
        lib->gates[i]= malloc(sizeof(gate));

        // find gate name by searching substring "COMP" and skiping it
        substring= "COMP ";
        ptr= strstr(line,substring) + strlen(substring);
        char* name= split(&ptr," ");

        // allocate space for the gate name and store it
        lib->gates[i]->name= malloc(strlen(name)+1);
        strncpy(lib->gates[i]->name,name,strlen(name)+1);
        
        // get the inputs raw string of the gate  
        char* input_raw= ptr + strlen("; IN:");

        char* truth_raw= strdup(ptr);
        //printf("truthbefore:%s",truth_raw);

        // printf("before: %s",ptr);
        // get the clear inputs string
        char* input= split(&input_raw," ;");
        // printf("after: %s",ptr);

        // first set inputs to NULL and let realloc do magic
        lib->gates[i]->inputs= NULL;

        //used in while
        int j=0;
        
        //temporary store the string
        char* temp=NULL;

        // read every gates inputs and stop when you cant find a "," meaning its the last input
        while(input!=NULL){
            //printf("%s",input);
            // find the ","
            temp= split(&input,",");

            // set the last characters of input to '\0' because they were \n earlier
            int len= strlen(temp);
            if(len!=1){
               temp[len-2]='\0';   
               temp[len-1]='\0';
            }

            // resigate  gates inputs and store every input
            lib->gates[i]->inputs= realloc(lib->gates[i]->inputs,sizeof(char*)*(i+1));
            lib->gates[i]->inputs[j]=malloc(strlen(temp)+1);
            strncpy(lib->gates[i]->inputs[j],temp,strlen(temp)+1);
            j++;
    
        }
        // store # of gates
        lib->gates[i]->num_inputs=j;
        
        //now lets get the truth table
                //printf("truthafter:%s",truth_raw);

        char* sub= " ; ";
        char* truth= strstr(truth_raw,sub) + strlen(sub);

        char* temp_truth= NULL;
        
        // used in while to find every truth table insertion
        int t=0;
        while(truth!= NULL){

            temp_truth= split(&truth,", ");

            // set the last characters of input to '\0' because they were \n earlier
            int len= strlen(temp_truth);
            if(len!=1){
               temp_truth[len-2]='\0';   
               temp_truth[len-1]='\0';
            }
            lib->gates[i]->truthtable[t]=atoi(temp_truth);

            t++;
        }
        lib->gates[i]->num_truthtable=t;
        
        


        i++;

    }

    lib->num_gates=i;

}

int main(){
    library* l= malloc(sizeof(library));

    int b=getLibraryFromFile("GATES.txt",l);

    //
    for( int i=0; i<l->num_gates; i++){
        printf("%s ",l->gates[i]->name);
        for(int j=0; j<l->gates[i]->num_inputs; j++)
        printf("%s ",l->gates[i]->inputs[j]);
        printf("%d",l->gates[i]->num_truthtable);
        for(int j=0; j<l->gates[i]->num_truthtable; j++){
            printf(" %d",l->gates[i]->truthtable[j]);
        }
        printf("\n");
    }
    
}