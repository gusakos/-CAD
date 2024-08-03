#include <stdlib.h>
#include <string.h>

#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>



typedef struct {
    int num_inputs;
    
   
    char* id;
    char* name;
    char** inputs; 
    int num_outputs; 
    char** outputs;
}Full_adder;

typedef struct {
    char* name;
    int num_inputs;
    int num_outputs;
    int num_bits;
    char** inputsA;
    char** inputsB;
    char** outputsS;
    char* outputCout;
    char* inputCin;
    char* lib;
    //char* add_subb
    Full_adder** adders;

}entity_adder;

void freeAdder(Full_adder* adder){

    for(int i=0; i<adder->num_inputs; i++){
         free(adder->inputs[i]);
    }
    free(adder->outputs[0]);
    free(adder->outputs[1]);
    free(adder->id);
    free(adder->name);
    free(adder);
   
}

void freeEntity(entity_adder* entity){
    free(entity->name);

    for(int i=0; i<entity->num_bits; i++){
        free(entity->inputsA[i]);
    }
    free(entity->inputsA);

    for(int i=0; i<entity->num_bits; i++){
        free(entity->inputsB[i]);
    }
    free(entity->inputsB);

    for(int i=0; i<entity->num_bits; i++){
        free(entity->outputsS[i]);
    }
    free(entity->outputsS);

    free(entity->outputCout);
    free(entity->inputCin);
    free(entity->lib);

    for(int i=0; i<entity->num_bits; i++){
        freeAdder(entity->adders[i]);
    }
    free(entity->adders);
    free(entity);

}



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

/*  Creates  FULL_ADDER Comp  
needs the inputs name and id(gate) to built the adder
free must be done by the caller*/
Full_adder* create_full_adder(char* input_A, char* input_B, char* input_C_in, char* name, int gate_number ) {

    // Creating a Full_adder instance 
    Full_adder *adder= malloc(sizeof(Full_adder));

    // Initializing adder's # of inputs,# of outputs and type                  
    adder->num_inputs = 3;
    adder->num_outputs=2;
    adder->name=strdup(name);

    //Creating  space for every input, output
    adder->outputs= malloc(sizeof(char*)*(adder->num_outputs));
    adder->inputs= malloc(sizeof(char*)*(adder->num_inputs));                         
    
    // Building strings needed to fill inputs , outputs and gate ID
    char* output_u1   = malloc(sizeof(char)*20);
    char* output_u2   = malloc(sizeof(char)*20);
    char* input_u= malloc(sizeof(char) * 4);
    snprintf(input_u,4,"u%d",gate_number);
    snprintf(output_u1,20,"u%d_S",gate_number);
    snprintf(output_u2,20,"u%d_Cout",gate_number);

    // Filling adder's outputs with Ui_S,Ui_Cout and gate number with ui
    adder->inputs[0] = strdup(input_A);                            
    adder->inputs[1] = strdup(input_B);
    adder->inputs[2] = strdup(input_C_in);
    adder->outputs[0]=strdup(output_u1);
    adder->outputs[1]=strdup(output_u2);
    adder->id = strdup(input_u);
    free(output_u1);
    free(output_u2);
    free(input_u);
    return adder;
}

/*creates an entity where every input and ouput is stored 
using a list of adders 
needs the name of the file and an entity* 
free must be done by the caller*/
int getEntity(char* file, entity_adder* entity){
    
     if (file == NULL )
    return 1;

    // set the lib to FULL_ADDER
    char* lib="FULL_ADDER";
    entity->lib=malloc(strlen(lib)+1);
    strncpy(entity->lib,lib,strlen(lib)+1);

    // new line will be stored here even if its set to null when getline is called
    char* line= NULL;
    // kepping track of the line we are currently in the file
    int offset=0;
    // needed for getline()
    size_t len= 0;
    // # of characters readen in new line to update the offset 
    int num_read=0;

    // read first line
    //read name
    num_read= read_line_from_file(&line,file,&len,offset);
    if(num_read==-1){
         printf("unable to read line ");
         exit(0);
     }

    // update offset
    offset+= num_read;
    
    //substring used to search temporary things in a line
    char* substring= "ENTITY ";

    // used to temporary point in the line
    char* ptr= strstr(line,substring)+ strlen(substring);

    // used to temp store a word 
    char* temp= split(&ptr," ");


    // store the name 
    entity->name= malloc(strlen(temp)+1);
    strncpy(entity->name,temp,strlen(temp)+1);

    // move to next line
    // get  the number of bits
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
    offset+= num_read;

    //get the number of bits and pass it to entity
    substring= "= ";
    ptr= strstr(line,substring)+ strlen(substring);
    temp= split(&ptr," ");
    entity->num_bits= atoi(temp);

    /*GIA ASKHSH5
    // move to next line
    // get  the LIB
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
    offset+= num_read;

    substring= "LIB ";
    ptr= strstr(line,substring)+ strlen(substring);
    temp= split(&ptr,"\n");
    entity->lib= malloc(strlen(temp)+1);
    strncpy(entity->lib,temp,strlen(temp)+1);
    
    */



    // move to next line
    //get the inputsA
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
    offset+= num_read;

    // find the inputs starting word by searching ": "
    substring= ": ";
    ptr= strstr(line,substring)+ strlen(substring);

    // make space for inputsA
    entity->inputsA= malloc(sizeof(char*)*entity->num_bits);

    //store every inputA to entity
    for(int i=0; i<entity->num_bits; i++){
        
        //split every input with delim " , "
        temp= split(&ptr," , ");
        if(i== entity->num_bits-1){
            ptr=temp;
            temp= split(&ptr," ");
        }
        entity->inputsA[i]= malloc(strlen(temp)+1);
        strncpy(entity->inputsA[i],temp,strlen(temp)+1);
    }
   
    //Exactly the same for inputs B

    // move to next line
    //get the inputsB
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
    offset+= num_read;

    // find the inputs starting word by searching ": "
    substring= ": ";
    ptr= strstr(line,substring)+ strlen(substring);

    // make space for inputsB
    entity->inputsB= malloc(sizeof(char*)*entity->num_bits);
    
    //store every inputB to entity
    for(int i=0; i<entity->num_bits; i++){

        //split every input with delim " , "
        temp= split(&ptr," , ");
        if(i== entity->num_bits-1){
            ptr=temp;
            temp= split(&ptr," ");
        }
        entity->inputsB[i]= malloc(strlen(temp)+1);
        strncpy(entity->inputsB[i],temp,strlen(temp)+1); 
    }
    
    // get next line
    // get Cin
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
    offset+= num_read;
    
    // find Cin word by searching ": {"
    substring= ": {";
    ptr= strstr(line,substring)+ strlen(substring);

    temp= split(&ptr,"}");


    //store Cin
    entity->inputCin= malloc(strlen(temp)+1);
    strncpy(entity->inputCin,temp,strlen(temp)+1);


    /*GIA ASKHSH 5
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
    offset+= num_read;


    substring= ": ";
    ptr= strstr(line,substring)+ strlen(substring);    

    temp= split(&ptr," ");



    entity->add_sub= malloc(strlen(temp)+1);
    strncpy(entity->add_sub,temp,strlen(temp)+1);

    */

    //move to next line
    //get outputs(Sums)
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
    offset+= num_read;

    substring= ": {";
    ptr= strstr(line,substring)+ strlen(substring);

    // make space for outputsS
    entity->outputsS= malloc(sizeof(char*)*entity->num_bits);
    
    //store every outputS to entity
    for(int i=0; i<entity->num_bits; i++){

        temp= split(&ptr," , ");
        if(i== entity->num_bits-1){
            ptr=temp;
            temp= split(&ptr," ");
        }
        entity->outputsS[i]= malloc(strlen(temp)+1);
        strncpy(entity->outputsS[i],temp,strlen(temp)+1); 
    }

    //move to next line
    //get Cout
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
    offset+= num_read;

    // find the outputs first word by searching ": {"
    substring= ": {";
    ptr= strstr(line,substring)+ strlen(substring);    

    temp= split(&ptr,"}");


    //store the outputs
    entity->outputCout= malloc(strlen(temp)+1);
    strncpy(entity->outputCout,temp,strlen(temp)+1);


    //Create every Full_adder component 
    entity->adders= malloc(sizeof(Full_adder*)*entity->num_bits);

    //first adder is unique cause third input is cin not Cout[i=1] so store it seperately
    entity->adders[0]=create_full_adder(entity->inputsA[entity->num_bits-1],entity->inputsB[entity->num_bits-1],entity->inputCin,entity->name,0);

    // Creates every other adder with the inputs collected by the entity
    for(int i=1; i<entity->num_bits; i++){

        entity->adders[i]= create_full_adder(entity->inputsA[entity->num_bits-(i+1)], entity->inputsB[entity->num_bits-(i+1)], entity->adders[i-1]->outputs[1],entity->name, i);

    }

    return 1;
}



/** Creates a table to store the netlist so that we can print it to a file
 * needs entity_adder* and char** netlist_table
 * free must be done by the caller
*/
int createTable (entity_adder* entity,char** netlist_table){
    // Create a table of strings to store the netlist
    // char **netlist_table = malloc(30 *5);

    // Add the netlist components to the table of strings  
    for (int i=0; i<(4+2*entity->num_bits+5); i++ ){
        netlist_table[i] = malloc(200*5);
    }
    

    //print first line COMP ; IN...
    int offset=snprintf(netlist_table[0],strlen("COMP  ; IN: ")+strlen(entity->name)+1,"COMP %s ; IN:",entity->name);
    int line_counter=0;

    //Print every inputA in first line
    for (int i=0; i<entity->num_bits; i++){
        offset+=snprintf(netlist_table[0]+offset,strlen(",")+strlen(entity->inputsA[i])+1,"%s,",entity->inputsA[i]);
        // we need to move line
    }

    //Print every inputB in first line
    for (int i=0; i<entity->num_bits; i++){
        offset+=snprintf(netlist_table[0]+offset,strlen(",")+strlen(entity->inputsB[i])+1,"%s,",entity->inputsB[i]);
    }

    //Print Cin in first line
    offset+=snprintf(netlist_table[0]+offset,strlen(";  ")+strlen(entity->inputCin),"%s; ",entity->inputCin);


    //Print every output in first line
    offset+=snprintf(netlist_table[0]+offset,strlen("OUT:")+1,"OUT:");

    // print every output
    for (int i=0; i<entity->num_bits; i++){
        offset+=snprintf(netlist_table[0]+offset,strlen(",")+strlen(entity->outputsS[i])+1,"%s,",entity->outputsS[i]);
    }

    // print Cout
    offset+=snprintf(netlist_table[0]+offset,strlen("\n")+strlen(entity->inputCin)+1,"%s\n",entity->outputCout);

    //print begin line
    snprintf(netlist_table[1],50,"BEGIN %s NETLIST\n",entity->name);

    //print every adder compont with this form Ui ADDER Input[0] Input[1] Input[2]
    for (int i=0; i<entity->num_bits; i++){

        snprintf(netlist_table[i+2],50,"%s %s %s %s %s \n",entity->adders[i]->id,entity->adders[i]->name,entity->adders[i]->inputs[0],entity->adders[i]->inputs[1],entity->adders[i]->inputs[2]);    
    }
    
    //print the outs ak the sums of every bit
    for(int i=0; i<entity->num_bits; i++){
        snprintf(netlist_table[2 + entity->num_bits + i],strlen("= \n   ")+strlen(entity->adders[i]->outputs[0])+strlen(entity->outputsS[i]),"%s= %s \n",entity->outputsS[entity->num_bits-(i+1)],entity->adders[i]->outputs[0]);
    }

    // print Cout
    snprintf(netlist_table[2 + 2*entity->num_bits],strlen("%s = %s\n"  )+strlen(entity->outputCout)+strlen(entity->adders[entity->num_bits-1]->outputs[1]),"%s = %s\n",entity->outputCout,entity->adders[entity->num_bits-1]->outputs[1]);
    
    //print End line
    snprintf(netlist_table[3+ 2*entity->num_bits],strlen("END %s NETLIST \n")+5+strlen(entity->name),"END %s NETLIST \n",entity->name);

    return 1;
}

int main(){
    int b=0;

    entity_adder* e= malloc(sizeof(entity_adder));

    b=getEntity("entity_adder.txt",e);

    // Create a table of strings to store the netlist
    char **netlist_table = malloc(30 *5);
    int a= createTable(e,netlist_table);


    // Open the output file for writing
    FILE *fp = fopen("netlistDo.txt", "w");
    if (fp == NULL) {
        printf("Error: Could not open file.\n");
        return 1;
    }

    // Write the netlist to the output file
    for (int i = 0; i < 4+ 2*e->num_bits; i++) {
        fprintf(fp, "%s", netlist_table[i]);
    }

    // Close the output file
    fclose(fp);

    //free the table
    for(int i=0; i<2*5+4; i++){
        free(netlist_table[i]);
    }
    free(netlist_table);
}