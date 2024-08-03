#include <stdlib.h>
#include <string.h>

#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#define GATE 1
#define SYBSYSTEM 2
#define GATES_COMP "COMPONENT LIBRARY"
#define FULL_ADDER_COMP "FULL_ADDER"
#define DELIM ";"
#define INS " IN:"
#define COMMA ","
#define OUTS " OUT:"
#define LIB "LIBRARY"


typedef struct {
    char* ID;
    int type;               
    char* name;
    int num_inputs;
    int num_outputs;
    char** inputs;
    char** outputs;
    

}component;

typedef struct {
    char* name;
    int num_inputs;
    char** inputs;

}gate;


typedef struct {
    int num_gates;
    gate** gates;
    char* file;
}library;

typedef struct {
    char* ID;
    char* name;
    int num_inputs;
    int num_outputs;
    char** inputs;
    char** outputs;
    int num_comps;
    component** components;
    char** outpout_mappings;
    char** mappings_comp;
    char** mappings_out;

}subsystem;


typedef struct {
    char** inputs;
    char** outputs;
    subsystem** adders;
    int num_inputs;
    int num_outputs;
    int num_adders;

}netlist;




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
        char* input= ptr + strlen("; IN:");

        // first set inputs to NULL and let realloc do magic
        lib->gates[i]->inputs= NULL;

        //used in while
        int j=0;
        
        //temporary store the string
        char* temp=NULL;

        // read every gates inputs and stop when you cant find a "," meaning its the last input
        while(input!=NULL){
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
        


        i++;

    }

     lib->num_gates=i;

}

void freeLibrary(library* lib){
    for(int i=0; i<lib->num_gates; i++){
        free(lib->gates[i]->name);
        for(int j=0; j<lib->gates[i]->num_inputs; j++){
            free(lib->gates[i]->inputs[j]);
        }
        free(lib->gates[i]->inputs);

        free(lib->gates[i]);
    }
    free(lib->gates);
    free(lib);
}

/*  gets a file cointaing a subsystem full_adder made of components , each component is a gate.
    Checks every component in the library of gates given and if there is a match store the component
    in the full_adder subsystem , if not the program exits and prints a message blaming the gate not found.
    Freein of Subsystem and each component must be done by the caller
    In order to create a full adder give the inputs in this form A,B,Cin*/
 int getSubSystemFromFile(char* file,char* def,subsystem* sub,library* l,char* id){
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


    // set the subsystem id
    sub->ID= malloc(strlen(id)+1);
    strncpy(sub->ID,id,strlen(id)+1);

    //set the subsystem name
    sub->name= malloc(strlen(def)+1);
    strncpy(sub->name,def,strlen(def)+1);

    // read first line
    num_read= read_line_from_file(&line,file,&len,offset);
    if(num_read==-1){
         printf("unable to read line ");
         exit(0);
     }

    // update offset
    offset+= num_read;

    // chech if its indeed the full_adder comp file
    char* substring= def;
    char* ptr= strstr(line,substring)+ strlen(substring) +1;
    if (ptr == NULL) {
    
        printf("Component Library not found.\n");
        exit(-1);
    }

    // get the inputs raw string
    char* inputs= split(&ptr,DELIM);

    // move to get rid of the unused chars
    inputs= inputs + strlen(INS);
    int _num_inps= strlen(inputs)/2 +1/2;

    // put every input into the subsystem struct using strtolist which allocates memory for the inputs array and each input too
    // inputs is set to null first in order for realloc to do its magic
    sub->inputs= NULL;   
 
    //give the inputs you want to the full_adder
    //char* inps= strdup(inputs_given);
    sub->num_inputs=str_to_list(inputs,&(sub->inputs),COMMA);
    //printf("%s",sub.)
   
    //get the outputs string
    char* outputs= ptr + strlen(OUTS);

    // do the last characters from "\n" to '\0'
    int outputs_len= strlen(outputs);
    outputs[outputs_len-3]='\0';
    outputs[outputs_len-2]='\0';
    outputs[outputs_len-1]='\0';



    // put every output into the subsystem struct using strtolist which allocates memory for the outputs array and each output too
    // outputs is set to null first in order for realloc to do its magic
    sub->outputs= NULL; 
    sub->num_outputs=str_to_list(outputs,&(sub->outputs),COMMA);
    

    // skip begin full_adder line
    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
             printf("unable to read line ");
             exit(0);
         }
    // update offset
     offset+= num_read;

    // used to read new  component line until its NULL and while stops
    char* temp=NULL;

    //used in while 
    int i=0;
    
    // setcomponents to NULL first for realloc to do its job        
    sub->components= NULL;

    //used to temporary store a compponents string
    char* temp_comp;
    char* temp_ptr=NULL;
    //get every component of the full_adder and stop when there is a line containing "=" meaning we are at ouputs mappings and components are done 
    do{

        //read next component
        num_read= read_line_from_file(&line,file,&len,offset);
         // update offset
        offset+= num_read;

        //MAYBE NEEDS CHANGING TO STRNCPY
        temp_ptr= strdup(line);
        //malloc(strlen(line)+1);
        //strncpy(temp_ptr,line,strlen(temp_ptr)+1);
        //printf("%s",temp_ptr);
        // /ptr =strdup(line);
        

        //do the last charachters equal to '\0' cause to get rid of the '\n' character 
        int ptr_len= strlen(temp_ptr);
        //int ptr_len= strlen(ptr);
        temp_ptr[ptr_len-2]='\0';
        temp_ptr[ptr_len-1]='\0';

        //check if there is "=" which means there arent any more components to read, if there is indeed break
        temp=strstr(line,"=");
        if (temp!=NULL){
            break;
        }
        
        
        // resize the array of components and get space for each component readen
        sub->components= realloc(sub->components,sizeof(component*)*(i+1));
        sub->components[i]= malloc(sizeof(component));
        
        // find components id by spliting the string whith delim " "
        // get space for the components ID and store it 
        temp_comp= split(&temp_ptr," ");

        // store components id
        sub->components[i]->ID= malloc(strlen(temp_comp)+3);
        strncpy(sub->components[i]->ID,temp_comp,strlen(temp_comp)+1);

        // found the component gate name, need to check it there is one in  the library
        temp_comp= split(&temp_ptr," ");
        
        
        // check every gate's name in the library of gates to see if there is a match
        for(int j=0; j<l->num_gates; j++){

            // enter if  the gate is found in order to store the inputs
            // break for if found   found in order to avoid extra loops
            if(strcmp(temp_comp,l->gates[j]->name)==0){
                sub->components[i]->name=malloc(strlen(temp_comp)+5);
                strncpy(sub->components[i]->name,temp_comp,strlen(temp_comp)+5);

                // set the components inputs number to the gates inputs number
                sub->components[i]->num_inputs=l->gates[j]->num_inputs;
                
                //get space for the inputs
                sub->components[i]->inputs= malloc(sizeof(sub->components[i]->inputs)*l->gates[j]->num_inputs);
            
                // put every input of the component
                for(int p=0; p<l->gates[j]->num_inputs; p++){

                    //get the input
                    temp_comp= split(&temp_ptr,",");

                    int len=strlen(temp_comp);
               
              
                    //check if comp's input is an input of the subsystem first
                    // if yes components input will point to the subsystem's input
                    for(int n=0; n<sub->num_inputs; n++){
                        
                        if(strcmp(temp_comp,sub->inputs[n])==0){
                            sub->components[i]->inputs[p]=(sub->inputs[n]);
                        }
                    }

                    //check if comp's input is another comp already stored
                    //if yes the comp input will point to the subsystem's comp already stored
                    for(int n=0; n<(i+1); n++){
                       
                        if(strcmp(temp_comp,sub->components[n]->ID)==0){
                            sub->components[i]->inputs[p]=(sub->components[n]->ID);
                            
                        }
                           if(n==i && sub->components[i]->inputs[p]==NULL){
                        printf("error at comp:%s and input:%s the input cant be found",sub->components[i]->ID,temp_comp);
                       // exit(0);
                    }
                    }
         
                }
                break;
            }
            // if loop ends without gate found print message and exit
            if(j==l->num_gates-1   ){
                printf("Gate %s not Found with ID:%s",temp_comp,sub->components[i]->ID);
                exit(0);
            }
        }
        
        i++;
       
    }while(temp==NULL);
    sub->num_comps= i;
    
    // lets find the output mappings now
    // just to remember at the netlist of full_addernbits we  use the mappings_out and mappings_comp arrays
    sub->outpout_mappings= malloc(sizeof(sub->outpout_mappings)*sub->num_outputs);
    sub->mappings_comp= malloc(sizeof(sub->mappings_comp)*sub->num_outputs);
    sub->mappings_out= malloc(sizeof(sub->mappings_comp)*sub->num_outputs);

    for(int j=0; j<sub->num_outputs; j++){

        temp_comp= split(&temp_ptr,", ");

        //replace the \n character in the last mapping 
         int len= strlen(temp_comp);
  
        char* out_temp= split(&temp_comp,"=");



        char* final_map;


        //chech if output was defined earlier
        //if it is output will now point to the subsystems output defined earlier
        for(int i=0; i<sub->num_outputs; i++){
            if(strcmp(out_temp,sub->outputs[i])==0){
                sub->mappings_out[j]= (sub->outputs[i]);
                out_temp=sub->outputs[i];
                final_map= malloc(strlen(out_temp)*5);
                strcpy(final_map,out_temp);

                //found so break
                break;
            }
            if(i==(sub->num_outputs-1)){
                printf("Problem at mappings output:%s isnt defined in outputs",out_temp);
                exit(0);
            }
            
        }
        //chek if comp is another comp already stored
        // if it is comp will point to the comp already stored
        for(int i=0; i<sub->num_comps; i++){

            if(strcmp(temp_comp,sub->components[i]->ID)==0){
                sub->mappings_comp[j]=sub->components[i]->ID;
                temp_comp=sub->components[i]->ID;
                strcat(final_map,"=");
                strcat(final_map,temp_comp);

                //found so break loop
                break;
            }
            if(i==sub->num_comps-1){
                printf("problem with  output_mappings mapping:%s wasnt found",temp_comp);
                exit(0);
            }
            
        }

        // get space for every mapping and store it
        sub->outpout_mappings[j]= malloc(strlen(final_map)+1);
        strncpy(sub->outpout_mappings[j],final_map,strlen(final_map)+1);
        free(final_map);
  }//telos bigfor


}

void freeComponent(component * comp){
    free(comp->name);
    free(comp->ID);
    for(int i=0; i<comp->num_inputs; i++){
        free(comp->inputs[i]);
    }
    free(comp->inputs);

    for(int i=0; i<comp->num_outputs; i++){
        free(comp->outputs[i]);
    }
    free(comp->outputs);    
    free(comp);
}

void freeSubsystem(subsystem *s){
    
    free(s->ID);
    free(s->name);

    for(int i=0; i<s->num_inputs; i++){
        free(s->inputs[i]);
    }
    free(s->inputs);

    for(int i=0; i<s->num_outputs; i++){
        free(s->outputs[i]);
    }
    free(s->outputs);

    for(int i=0; i<s->num_comps; i++){
        freeComponent(s->components[i]);
    }
    free(s->components);

    for(int i=0; i<s->num_outputs; i++){
        free(s->outpout_mappings[i]);
    }
    free(s->outpout_mappings);
    
    for(int i=0; i<s->num_outputs; i++){
        free(s->mappings_comp[i]);
    }
    free(s->mappings_comp);

    for(int i=0; i<s->num_outputs; i++){
        free(s->mappings_out[i]);
    }
    free(s->mappings_out);
    
    free(s);
}



/* Reads the netlist file given by the user and creates full adders with the  inputs given
    First 2 inputs of every  are given by the file and are inpus of the netlist too (Ai Bi) but 
    the last input is the carry_out of the last adder passed by its gate ID
    Prints the netlist comprising only of gates  id,name,inputs.
    Free must be done by the caller*/
 int setNetlist(char* file, netlist* net, library* lib){
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

    // read first line
    num_read= read_line_from_file(&line,file,&len,offset);
    if(num_read==-1){
         printf("unable to read line ");
         exit(0);
    }

    // update offset
    offset+= num_read;


    // move the ptr to the inputs raw string
    char* substring= INS;
    char* ptr= strstr(line,substring)+ strlen(substring);
    if (ptr == NULL) {  
      printf("Component Netlist not found.\n");
      exit(-1);
    }


    //set the inputs of the netlist
    char* inputs= split(&ptr,"; ");

    net->inputs=NULL;
    net->num_inputs=str_to_list(inputs,&(net->inputs),COMMA);

    
    //set the ouputs of the netlist
    char* temp= split(&ptr,"OUT:");

    //do the last charachters equal to '\0' cause to get rid of the '\n' character 
    int ptr_len= strlen(ptr);
    ptr[ptr_len-1]='\0';
    net->outputs=NULL;
    net->num_outputs=str_to_list(ptr,&(net->outputs),COMMA);
 
    // read next line and skip it
    num_read= read_line_from_file(&line,file,&len,offset);
    if(num_read==-1){
         printf("unable to read line ");
         exit(0);
    }
    // update offset
    offset+= num_read;


    net->adders= NULL;

    
    // temporary store the inputs here
    char** inps= malloc(sizeof(char*)*30);
    for(int p=0; p<30; p++){
        inps[p]=malloc(sizeof(char*));
    }


    temp=NULL;

    char* temp_adder;

    int counter=0;  // counter of items in inps 

    char buffer[10];

    
    int i=0;        //adders counter
    do{
        // get next line aka the next adder
        num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
        printf("unable to read line ");
        exit(0);
        }
        // update offset
        offset+= num_read;


        //check if there is "=" which means there arent any more components to read, if there is indeed break
        temp=strstr(line,"=");
        if (temp!=NULL){
            break;
        }

        // inps= realloc(inps,sizeof(char*)*3*(i+1));

        ptr =strdup(line);
        
        //do the last charachter equal to '\0' cause to get rid of the '\n' character 
        int ptr_len= strlen(ptr);
        ptr[ptr_len-1]='\0';

  

        // resize the array of adders and get space for each adder readen
        net->adders= realloc(net->adders,sizeof(subsystem*)*(i+1));
        net->adders[i]= malloc(sizeof(subsystem));


        // lets get the adder's ID from the line stored in ptr
        char* id;
        id=split(&ptr," ");
       

        // check if its indeed a full adder comp and move the substring into the inputs
        ptr= strstr(line,FULL_ADDER_COMP)+ strlen(FULL_ADDER_COMP)+1;
        if (ptr == NULL) {        
            printf("Full adder not found.\n");
            exit(-1);
        }

        //get rid of the "\n" character at the end of the string
        //now ptr is pure inputs of a full adder
        ptr_len= strlen(ptr);
        ptr[ptr_len-1]='\0';
        

        // first store the the frist full adder with hard coded ID=0 inputs A0,B0,Cin
        // we need to store hhim only once so if i=0 store it
        if(i==0){
            getSubSystemFromFile("Comp_full_adder.txt",FULL_ADDER_COMP,net->adders[0],lib,id);
           
        }
        else{
            // now define every other adder, to be precise here we only make the adder and the connections between the inputs and ouputs
            // originals inputs will be given later
            getSubSystemFromFile("Comp_full_adder.txt",FULL_ADDER_COMP,net->adders[i],lib,id);
            for(int j=0; j<net->adders[i]->num_comps; j++){

                sprintf(buffer,"U%d",j+(5*i));
                //printf("%s",buffer);
                strcpy(net->adders[i]->components[j]->ID,buffer);
                //printf("%s",net->adders[i]->components[j]->ID);


            }
        }
        //Now give the original inputs
        //now we need to get every  input of the full adder ,check it is a netlist input or an already stored full adder carry out
        // if it is a netlist input we need the full adder input to point to the netlist input
        // if it is a carry out of an already stored full adder , the full addder input should point to the carry out
        int k=0;
        do{
            temp_adder=split(&ptr,", ");

            int found=0;
            // check first if it is an input ,if found break            
            for(int j=0; j<net->num_inputs; j++){            
                if(strcmp(temp_adder,net->inputs[j])==0){
                    counter++;
                    inps[counter]= net->inputs[j];
                    found=1;
                    break;
                }
            }
            if(found==0){

                char* temp_cout= malloc(strlen(temp_adder)+1);
                temp_cout= split(&temp_adder,"_COUT");
            
                
                //chek if it is a carry out
                 for(int j=0; j<(i); j++){  // for every adder already stored

                    if(strcmp(net->adders[j]->ID,temp_cout)==0){ // get the carry out mapping of the adder identified
                        counter++;
                        inps[counter]= net->adders[j]->mappings_comp[1];

                        break;
                    }
                }
            }
            
             
            k++;           

        }while(ptr!=NULL);
       
    // STORE FCKN FINALLY THE INPUTS
    strcpy(net->adders[i]->inputs[0],inps[counter-2]);
    strcpy(net->adders[i]->inputs[1],inps[counter-1]);
    strcpy(net->adders[i]->inputs[2],inps[counter]);


    // PRINT NETLIST
    for(int j=0; j<net->adders[i]->num_comps; j++){

        printf("%s %s %s,%s\n",net->adders[i]->components[j]->ID,net->adders[i]->components[j]->name,net->adders[i]->components[j]->inputs[0],net->adders[i]->components[j]->inputs[1]);
        
    }
    printf("%s%d=%s\n",net->adders[i]->mappings_out[0],i,net->adders[i]->mappings_comp[0]);

    i++;
    }while(temp==NULL);
net->num_adders=i;
    
printf("Cout=%s",net->adders[i-1]->mappings_comp[1]);

  
for(int j=0; j<30; j++){
    free(inps[j]);
}
free(inps);


}


void freeNetlist(netlist* net){
    for(int i=0; i<net->num_inputs; i++){
        free(net->inputs[i]);
    }
    free(net->inputs);

    for(int i=0; i<net->num_outputs; i++){
        free(net->outputs[i]);
    }
    free(net->outputs);

    for(int i=0; i<net->num_adders; i++){
        freeSubsystem(net->adders[i]);
    }
    free(net->adders);
    free(net);
}


int main(){
    int b;

    library* l= malloc(sizeof(library));

    b=getLibraryFromFile("GATES.txt",l);



    netlist* n= malloc(sizeof(netlist));
    int c;
    c= setNetlist("netlist.txt",n,l);

    
    freeLibrary(l);
    printf("\n");
    //freeSubsystem(s);
    //freeNetlist(n);
}





