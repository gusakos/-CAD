#include <stdlib.h>
#include <string.h>

#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#define COMP_DES "U"
#define ID_DES " "
#define GATES_COMP "COMPONENT LIBRARY"
#define FULL_ADDER_COMP "FULL_ADDER"
#define FULL_SUB_ADDER_COMP "FULL_SUBTRACTOR"
#define DELIM ";"
#define INS " IN:"
#define COMMA ","
#define OUTS " OUT:"
#define LIB "LIBRARY"
#define IN "IN"
#define OUT "OUT"




/** Every gate containing the name ,the inputs and the values of the truthtable
 * to look for when needed
 * Its basically the library where we refer to to do the calculationg for eeach gate
*/
typedef struct {
    char* name;                     //gate name
    int num_inputs;                 // #inputs
    char** inputs;                  // input's IDs
    int  truthtable[4];             // truth table's values , max4
    int num_truthtable;             // # truth table's values 2 or 4
}gate;

/**Every component with its inputs IDs and values
 * In order to calculate the output value get access to the gate's
 * truth table via a reference gate*
*/
typedef struct {
    char* ID;                       // Comp's ID
    char* name;                     //Comps Name
    int num_inputs;                 // #inputs
    char** inputs;                  // input's IDs
    int input_values[2];            // inputs of the component , max 2
    gate* refernce;                 // used to refer to the correct gate from the library and have acces to the ,truth table,num inputs etc..
    int num_dependent;              // NOT USED // keep count if the comp ouput is dependent on previous comp ouputs and is a critical path
    char* dependent[2];             //NOT USED // store every input dependent on previous components so you know its max 2
    int output;//                   // output's value of component
}component;

/**Subsystem's sturct containing every inputs and outputs and components given by the subsystem's file readen by getSubSystemFromFile()
 * At the end of calculation done by doubleBufferSystem() every value of each component is saved 
 * Every susystem's output value is saved too using findTheSubsOutputs()
 * Every testbench output is matched and saved by findTestbenchOutputs()
*/
typedef struct {
    char* ID;                       // subystem's ID NOT USED   
    char* name;                     // subsystem's name NOT USED
    int num_inputs;                 //# inputs of the subsystem file            
    int num_outputs;                //# outputs of th subsystem file
    int num_test_outputs;           //Outputs given by the testbench
    char** inputs;                  // A1=1 B2=0 bust just the strings  A1, B2
    int* input_values;              // A1=1 B2=0 but just the numbers 1 ,0 only one value for each input
    char** outputs;                 // outputs names given from testbench
    int* output_values;             //outputs values of subystem
    int* output_test_values;        // output's of testbench values
    int num_comps;                  //# components
    component** components;         // array of components
    int** all_inputs_values;        //every number of the input A and B and etc , they are saved with the order of the inputs given
    int* all_inputs_values_counter; //counter of the numbers of the input given , they are saved ith the orderd of th inputs given
    char** mappings_comp;           // //U43 given from subsystem file
    char** mappings_out;            //S= given from subsystem file 

}subsystem;


/** Contains every gat given from the file readen bygetLibraryFromFile()*/
typedef struct {
    int num_gates;                  // # gates
    gate** gates;                   // gates array
    char* file;                     //name of the file given
}library;



/**Compares 2 strings with strncmp and returns 1 if the fisrt string starts with the second string
 * if not returns 0
*/
int checkStart(char *s1, char *s2) {


    if (strncmp(s1, s2, strlen(s2)) == 0) 
    return 1;
    
    return 0;
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
even if line is null getline() allocates the need space for it
if it cant read returns -1*/
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
   every gate has a truth table which we ll need in order to perform the caclulations
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

    // read first line and skip
     num_read= read_line_from_file(&line,file,&len,offset);
    if(num_read==-1){
        printf("unable to read line ");
        exit(0);
    }
    // update offset
    offset+= num_read;

    // read second line and chech if it is indeed the library
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

        // save the truth table raw string in orfer to use later
        char* truth_raw= strdup(ptr);

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

        // save the clean truth table string
        char* truth= strstr(truth_raw," ; ") + strlen(" ; ");

        char* temp_truth= NULL;
        
        // used in while to find every truth table insertion
        int t=0;

        //if truth== NULL exit witch means delim " ;" wasnt found so there are no more insesrtions to search
        while(truth!= NULL){
            
            // find each insertion to the truth table
            temp_truth= split(&truth,", ");

            // set the last characters of input to '\0' because they were \n earlier
            int len= strlen(temp_truth);
            if(len!=1){
               temp_truth[len-2]='\0';   
               temp_truth[len-1]='\0';
            }
            // set the value
            lib->gates[i]->truthtable[t]=atoi(temp_truth);

            t++;
        }

        // save the number of insertions in a truth table
        lib->gates[i]->num_truthtable=t;
        
        


        i++;

    }
    // set the number os insertions in the truth table
    lib->num_gates=i;

}

/** reads a file containing a subsystem and sets its values
 * if a line starts with u it means its a component definition
 * for every component we save its inputs id and a pointer to the gate found in the library to keep track of its truth table
 * otherwise the line is an output definition
 * outputs are splitted into two strings  
 * the mappings_out whitch is the name of the output signal S= 
 * and the mappings_comp whitch is the component's value the output looks to =U3
*/
int getSubSystemFromFile(char* file,subsystem* sub,library* l){
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

    // we ll use ptr to temporary store the line
    char* ptr;

    num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
            printf("unable to read line ");
            exit(0);
        }

    // used in while to count the #components
    int i=0;

    //used in while to count outputs
    int m=0; 
    
    // setcomponents to NULL first for realloc to do its job        
     sub->components= NULL;

    // set the mappings to NULL first for realloc to do its job        
    sub->mappings_comp= NULL;
    sub->mappings_out= NULL;

    //read every line until the end of the file
    while(num_read!=-1){
        
        // move to next line
        num_read= read_line_from_file(&line,file,&len,offset);
        if(num_read==-1){
            break;
        }

        // update offset
        offset+= num_read;

        if(num_read==-1){
            printf("why");
            break;
        }

        //store the line here 
        ptr=strdup(line);

        
        

        //used to temporary store a compponents string
        char* temp_comp=NULL;

        // check if line starts with "U" then is a new component
        if(checkStart(line,COMP_DES)){
            
            
            
            
            // first get the ID of the component like "U1" by spliting with " "   
            char* temp_comp= split(&ptr,ID_DES);

            // resize the array of components and get space for each component readen
            sub->components= realloc(sub->components,sizeof(component*)*(i+1));
            sub->components[i]= malloc(sizeof(component));
            
            // find components id by spliting the string whith delim " "
            
            // get space for the components ID and store it 
            sub->components[i]->ID= malloc(strlen(temp_comp)+1);

            //store the component id
            strncpy(sub->components[i]->ID,temp_comp,strlen(temp_comp)+1);

            // found the component gate name, need to check it there is one in  the library
            temp_comp= split(&ptr," ");

            sub->components[i]->num_dependent=0;
            // check every gate's name in the library of gates to see if there is a match
            for(int j=0; j<l->num_gates; j++){

                // enter if  the gate is found in order to store the inputs
                // break for if found   found in order to avoid extra loops
                if(strcmp(temp_comp,l->gates[j]->name)==0){
                    sub->components[i]->name=malloc(strlen(temp_comp)+1);
                    strncpy(sub->components[i]->name,temp_comp,strlen(temp_comp)+1);

                    //printf("%s",sub->components[i]->name);

                    //set the gate refernce of the component to the gate of the library found
                    sub->components[i]->refernce=l->gates[j];

                    //printf("%d",sub->components[i]->refernce->truthtable[0]);

                    // set the components inputs number to the gates inputs number
                    sub->components[i]->num_inputs=l->gates[j]->num_inputs;
                    
                    //get space for the inputs
                    sub->components[i]->inputs= malloc(sizeof(sub->components[i]->inputs)*l->gates[j]->num_inputs);
                
                    // put every input of the component
                    for(int p=0; p< sub->components[i]->num_inputs; p++){

                        //get the input
                        temp_comp= split(&ptr,", ");


                        // we need to get rid of the "\n" character and replace it with '\0'
                        int len=strlen(temp_comp);
                        if(p==sub->components[i]->num_inputs-1){
                            temp_comp[len-2]='\0';
                        }

                        //allocate space for the input
                        sub->components[i]->inputs[p]=malloc(sizeof(strlen(temp_comp))+1);

                        //store the input
                        strncpy(sub->components[i]->inputs[p],temp_comp,strlen(temp_comp)+1);

                    

                        // // //check if comp's input is another comp already stored
                        // // //if yes increase the number of dependent of the componenent and save the dependet input
                        // for(int n=0; n<i; n++){

                        //     //check if it is a previous component
                        //     if(strcmp(temp_comp,sub->components[n]->ID)==0){
                        //         //increase the counter
                        //         sub->components[i]->num_dependent++;
                        //         //save the dependent
                        //         if(sub->components[i]->num_dependent==1){
                        //             sub->components[i]->dependent[1]=malloc(sizeof(strlen(temp_comp)));
                        //             strncpy(sub->components[i]->dependent[1],temp_comp,strlen(temp_comp));
                        //         }
                        //         //save the dependent
                        //         else{
                        //              sub->components[i]->dependent[2]=malloc(sizeof(strlen(temp_comp)));
                        //             strncpy(sub->components[i]->dependent[2],temp_comp,strlen(temp_comp));
                        //         }
                        //         break;
                                
                        //     }
                        
                        // }
            
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
        }
        //else it is just an output 
        else{
            // resize the arrays of mappings
            sub->mappings_comp= realloc(sub->mappings_comp,sizeof(sub->mappings_comp)*(m+1));
            sub->mappings_out= realloc(sub->mappings_out,sizeof(sub->mappings_out)*(m+1));

            // first dinf the output aka the signal
            char* out_temp= split(&ptr," =");

            //get space for the ouput
            sub->mappings_out[m]= malloc(strlen(out_temp)+1);
            
            //store the output
            strncpy(sub->mappings_out[m],out_temp,strlen(out_temp)+1);
           // printf("%s",sub->mappings_out[m]);
            
            //now we need to store the component_out aka the value the output takes
            //move one to get rid of the " " character
            ptr=ptr+1;

            // we need to get rid of the "\n" if ti exists character and replace it with '\0' 
            //aka if its not the last line

            //check if there is a '\n'
            out_temp= split(&ptr,"\n");

            //if there is replace it with '\0' 
            if(ptr!=NULL){
                int len=strlen(out_temp);
                out_temp[len-1]='\0';
            }
            
            //get space for the mappings comp
            sub->mappings_comp[m]= malloc(strlen(out_temp)+1);

            //store the mappings comp
            strncpy(sub->mappings_comp[m],out_temp,strlen(out_temp)+1);
           // printf("%s",sub->mappings_comp[m]);


            m++;
        }
        
        
    }
    sub->num_comps=i;
    sub->num_outputs=m;
    sub->output_values= malloc(sizeof(sub->output_values)*sub->num_outputs);

}

/** Reads a testbench file
 * saves every input name in the inputs name
 * saves every input's values in a all_inputs 2D array 
 * saves the # values of every input in all_inputs_counter array
 * saves an output name in the outputs array
 * OUT or IN can be given with the form we want either IN is first or second it doesnt matter
*/
int readTestbench(char* file,subsystem** sub){
     if (file == NULL )
        return 1;
    
    // new line will be stored here even if its set to null when getline is called
    char* line= NULL;

    // kepping track of the line we are currently in the file
    int offset=0;

    // needed for getline()
    size_t len= 0;

    //used to temporary store a line 
    char* ptr;

    // # of characters readen in new line to update the offset 
    int num_read=0;


    //used to temporary store a string in order to save it somewhere
    char* temp=NULL;


    

    //set inputs to null in order to realloc them 
    (*sub)->inputs= NULL;

    //set all inputs values to null in order to realloc them 
    (*sub)->all_inputs_values= NULL;
    
    //set all inputs values counter to 0
    (*sub)->all_inputs_values_counter= 0;

    //set outputs to null in order to realloc them 
    (*sub)->outputs= NULL;

    //used to count the inputs
    int i=0;

    // used to count the outputs
    int u=0;

    while(num_read!= -1){

        // read first line
        num_read= read_line_from_file(&line,file,&len,offset);
                
        
        // update offset
        offset+= num_read;

        // check it its the end of the file
        if(num_read==-1){
            break;
        }
        

        // check if line starts with "IN" then we are getting the outputs
        if(checkStart(line,IN)){

            //for evry input
            while(num_read!= -1){

                // move to next input line
                num_read= read_line_from_file(&line,file,&len,offset);
               
                
                    
                //store the line here 
                ptr=strdup(line);

                //check if its the end of the inputs and the outputs start now
                if(checkStart(line,OUT)){
                   break;
                }
                // check it its the end of the file
                if(num_read==-1){
                    break;
                }

                
                 // update offset
                offset+= num_read;

                // get rid of the \n character at the end of the lind
                int ptr_len= strlen(ptr);
                ptr[ptr_len]='\0';

                // now we need to take the inputs name and store it to the inputs list
                
                // resize the array of inputs and get space for each input readen
                (*sub)->inputs= realloc((*sub)->inputs,sizeof((*sub)->inputs)*(i+1));

                

                //get the name of the input
                temp=split(&ptr," ");
                
                //get space for each input readen
                (*sub)->inputs[i]= malloc(strlen(temp)+1);
                
                //save the input 
                strncpy((*sub)->inputs[i],temp,strlen(temp)+1);

                // now lets save every input values
                //realloc the input_values array for the new input
                (*sub)->all_inputs_values= realloc((*sub)->all_inputs_values,sizeof((*sub)->all_inputs_values)*(i+1));
                
                //realloc the input_values counters array for the new input
                (*sub)->all_inputs_values_counter= realloc((*sub)->all_inputs_values_counter,sizeof((*sub)->all_inputs_values_counter)*(i+1));

                //set the input's values aray to nULL in order to realloc it
                (*sub)->all_inputs_values[i]=NULL;

                // is used to count the values of every input
                int v=0;

                // for every value of this input
                while(ptr!=NULL){

                    //get every input value
                    temp=split(&ptr,", ");

                    // realloc the array of values of this input
                    (*sub)->all_inputs_values[i]= realloc((*sub)->all_inputs_values[i],sizeof((*sub)->all_inputs_values[i-1])*(v+1));

                    //save this value of ths input
                    (*sub)->all_inputs_values[i][v]= atoi(temp);

                    // counter of inputs values++
                    v++;
                }
                //save the counter of values for this input
                (*sub)->all_inputs_values_counter[i]=v;
                
                // counter of inputs ++
                i++;

            }
            //save the counter of inputs
            (*sub)->num_inputs=i;
            


        }
        else if(checkStart(line,OUT)){
             //for evry input
            while(num_read!= -1){

                // move to next input line
                num_read= read_line_from_file(&line,file,&len,offset);
               
                    
                //store the line here 
                ptr=strdup(line);

                //check if its the end of the inputs and the outputs start now
                if(checkStart(line,IN)){
                   break;
                }
                // check it its the end of the file
                if(num_read==-1){

                    break;
                }

                // update offset
                offset+= num_read;

                // get rid of the \n character at the end of the lind
                int ptr_len= strlen(ptr);
                
                //set it to '\0'
                ptr[ptr_len-2]='\0';

                // now we need to store the outputs names


                // resize the array of outputs and 
                (*sub)->outputs= realloc((*sub)->outputs,sizeof((*sub)->outputs)*(u+1));
                
                // //get space for each output readen
                (*sub)->outputs[u]= malloc(strlen(ptr)+1);

                //store the output name
                strncpy((*sub)->outputs[u],ptr,strlen(ptr)+1);
                //++ output's countr
                u++;
            }
            //save the outputs counter
            (*sub)->num_test_outputs=u;
        }
        //save the inputs counter
        (*sub)->num_inputs=i;
        
        //save the outputs counter
        (*sub)->num_test_outputs=u;
    }
    
        
       

    
}

/**creates the 2 buffers and initialize their values to -1= 'X'
 * creates a temoprary buffer
 * freeing buffers must be done by the caller
*/
void createBuffers(int** buffer1, int** buffer2, int** temp_buffer_input,int** temp_buffer_output , int size) {
    *buffer1 = (int*)malloc(size * sizeof(int));
    *buffer2 = (int*)malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        (*buffer1)[i] = -1;
        (*buffer2)[i] = -1;
    }
    *temp_buffer_input = (int*)malloc(size * sizeof(int));
    *temp_buffer_output = (int*)malloc(size * sizeof(int));

}

/**For every component of the subsystem we check every input and give the correct value of the input
 * if the input is a component it is set with the value of the component in the buffer
 * if it is a subsystem input it is set to the value of the subsystem input
*/
int setInputs(int* inputbuffer, subsystem** sub){
    // for every component in the subsystem 
    for(int i=0; i<(*sub)->num_comps; i++){

        
        //for every input of the component
        for(int j=0; j<(*sub)->components[i]->num_inputs; j++){
            
            // first set the value to -1
            (*sub)->components[i]->input_values[j]= -1;
            
            //check every subsystem's input 
            for(int k=0; k<(*sub)->num_inputs; k++){
                //if it is a match set the component's input= subsystem's input
                // break no need to check the other subsystem's inputs for a match
                if(strcmp((*sub)->components[i]->inputs[j],(*sub)->inputs[k])==0){
                    (*sub)->components[i]->input_values[j]=(*sub)->input_values[k];
                    //printf("no\n");
                    //printf("%s %s",(*sub)->components[i]->inputs[j],(*sub)->inputs[k]);
                    break;
                }               
            }
            //check every components id's
            for(int k=0; k<(*sub)->num_comps; k++){
                //if it is a match set the components input= previous component output
                if(strcmp((*sub)->components[i]->inputs[j],(*sub)->components[k]->ID)==0){
                    (*sub)->components[i]->input_values[j]= inputbuffer[k];
                    //printf("yes\n");
                    break;
                }
            }
            
        }

    }

}

/** makes the calculation by reading the inputs and checking the value from the truthtable of the gate
 * The truth table is given by the reference to a gate
 * Take the inputs values from th input buffer
 * Write the output to the output buffer
 * Return a counter with the values that changed
*/
int doTheMath(int** inputbuffer, int** output_buffer, subsystem* sub){
    //keeps track of the different values between the privious output and the present output
    int counter=0;
    for(int i=0; i<sub->num_comps; i++){    
        

        //check the inputs and give the correct value of the truth table

        //gate with 4 possible outputs
        if(sub->components[i]->refernce->num_truthtable==4){
            //case of 0,0
            if(sub->components[i]->input_values[0]==0 && sub->components[i]->input_values[1]==0){
                //set the output value in the output buffer
                (*output_buffer)[i]=sub->components[i]->refernce->truthtable[0];

            }
            //case of 0,1
            else if(sub->components[i]->input_values[0]==0 && sub->components[i]->input_values[1]==1){
                //set the output value in the output buffer
                (*output_buffer)[i]=sub->components[i]->refernce->truthtable[1];
            }
            //case of 1,0
            else if(sub->components[i]->input_values[0]==1 && sub->components[i]->input_values[1]==0){
                //set the output value in the output buffer
                (*output_buffer)[i]=sub->components[i]->refernce->truthtable[2];
            }
            // case of 1,1
            else if(sub->components[i]->input_values[0]==1 && sub->components[i]->input_values[1]==1){
                //set the output value in the output buffer
                (*output_buffer)[i]=sub->components[i]->refernce->truthtable[3];
            }
            //case an input isnt definied yet
            else if(sub->components[i]->input_values[0]==-1 || sub->components[i]->input_values[1]==-1){
                // do nothing
            }
        }
        else{
            //case 0 
            if(sub->components[i]->input_values[0]==0){
                //set the output value in the output buffer
                (*output_buffer)[i]=sub->components[i]->refernce->truthtable[0];
            }
            // case of 1
            else if(sub->components[i]->input_values[0]==1){
                //set the output value in the output buffer
                (*output_buffer)[i]=sub->components[i]->refernce->truthtable[1];
            }
            // case input isnt defined yet
            else{
                // do noting
            }
        }
        if((*output_buffer)[i]!=(*inputbuffer)[i]){
            counter++;
        }
        printf("%s:%d\n",sub->components[i]->ID,(*output_buffer)[i]);
    }
    printf("\n");
    return counter;
}

/** check the outputs of the subsystem and the components saved in order to find the value of each output
 * get the components outputs and the subsystem's outputs
 * for every mappings_comp find the correct component
 * and set the outuput's value= components.output
*/ 
int findTheSubsOutputs( subsystem** sub){
    char* temp;

    // first we need to find the output component
    for(int i=0; i<(*sub)->num_outputs; i++){
        
        

        // seach for every component to map it
        for(int j=0; j<(*sub)->num_comps; j++){
            
            //found the output value
            if( strcmp((*sub)->components[j]->ID,(*sub)->mappings_comp[i])==0 ){
                (*sub)->output_values[i]=(*sub)->components[j]->output;
                
                break;
            }
            // if component is not found set the value to -1
            if(j== (*sub)->num_comps-1){
                (*sub)->output_values[i]=-1;
                // print waarning message
                printf("Subsystem cant really converge because the output_mapping:%s of the output:%s cant be found",(*sub)->mappings_comp[i],(*sub)->mappings_out[i]);
            }
        }
        
    }
}

/** check the last output_buffer and give every value of it to every component's output
 * 
*/
int saveLastValues( subsystem** sub , int* output_buffer ){

    // for every component in the subsystem 
    for(int i=0; i<(*sub)->num_comps; i++){

        (*sub)->components[i]->output= output_buffer[i];        

    }
} 

/** Do the whole math
* At first buffer1 is the input buffer and buffer 2 is th output
* for every new iteration of calculations change the 2 buffers via a temp_buffer
*/
int doubleBufferSystem(subsystem* s){

    // temporary buffer in order to chang the input output buffer
    int* buffer1;
    int* buffer2;


    // original input/output buffers to give to the functions
    int* buffer_input;
    int* buffer_output;
    int bufferSize = s->num_comps;

    //create the buffers and initialize values to -1= 'X'
    createBuffers(&buffer_input, &buffer_output,&buffer1, &buffer2, bufferSize);

    //keep track if the 2 buffers values are different 
    // if not it means we dont have to iterate any more
    int a=1;

    //keep track of the iterations made aka how many cycles we had to to in order for the system to be stable
    int iterations_num=0;

    //keep interating until there are no changes in the buffer
    while(a>0){
        //read the inputs 
        a=setInputs(buffer_input,&s);

        // do the calculation using th inputs and the truthtable
        a=doTheMath(&buffer_input,&buffer_output,s);

        //temporary store  the 2 buffers 
        buffer1= buffer_input;
        buffer2= buffer_output;
        
        //change the 2 bufferss
        buffer_input=buffer2;
        buffer_output=buffer1;

        //++ the iterations counter
        iterations_num++;
    

    }
    // now that the finals values are set we need to save them to the subsystem
    //save every components output for them output buffer
    saveLastValues(&s,buffer_output);

    
    return iterations_num;

}

/**search every output asked in the testbench
 * search every ouput of the subystem if its a match
 * search every component id if its a match
 * search every input id if it is a match
*/
int findTestbenchOutputs(subsystem** sub){
    (*sub)->output_test_values= malloc(sizeof(int)*(*sub)->num_test_outputs);
    //for every output
    for(int i=0; i<(*sub)->num_test_outputs; i++){

        //first initialize it to -1
        (*sub)->output_test_values[i]=   -1;

        //search every subs mapping_out first
        for(int j=0; j<(*sub)->num_outputs; j++){

            //output found in the subs outputs
            if(strcmp((*sub)->mappings_out[j],(*sub)->outputs[i])==0){

                //set the value
                (*sub)->output_test_values[i]=(*sub)->output_values[j];

                //no need for othe iterations
                break;
                //printf("%s:%d",(*sub)->outputs[i],(*sub)->output_test_values[i]);
            }
        }

        //search every component
        for(int j=0; j<(*sub)->num_comps; j++){
            
            //output found in one components id
            if(strcmp((*sub)->components[j]->ID,(*sub)->outputs[i])==0){

                //set value
                (*sub)->output_test_values[i]= (*sub)->components[j]->output;
               // printf("%s:%d",(*sub)->outputs[i],(*sub)->output_test_values[i]);

                //no need for othe iterations
                break;

            }
        }

        //search every input
        for(int j=0; j<(*sub)->num_inputs; j++){

            //output found in one of inputs
            if(strcmp((*sub)->inputs[j],(*sub)->outputs[i])==0){

                 //set value
                (*sub)->output_test_values[i]= (*sub)->input_values[j];

                //no need for othe iterations
                break;
            }
        }
        printf("%s:%d\n",(*sub)->outputs[i],(*sub)->output_test_values[i]);

    }
}

/** Runs the whole program!!
 * Does the calculation for every value of each input and prints the output in the file
 * basically just puts the correct inputs in th subsystem
 * and call doubleBufferSystem to do the rest
 * finally calls findTestbenchOutputs to fill the outputs given
*/
int doForEveryValue(subsystem** sub){
    //we need to cal double buffer system for every value of eac output
    //we already know the number of values for each output 
    //so we will call them one at a time
    
    (*sub)->input_values= malloc(sizeof(int*)*(*sub)->num_inputs);

    // Open the output file for writing
    FILE *fp = fopen("FINAL.txt", "w");
    if (fp == NULL) {
        printf("Error: Could not open file.\n");
        return 1;
    }
    //print first line of the file
    fprintf(fp,"INPUTS | OUTPUTS\n");

    //print  every input
    for(int i=0; i<(*sub)->num_inputs; i++){
        fprintf(fp,"%s ",(*sub)->inputs[i]);
    }
    //print |
    fprintf(fp,"| ");

    //print every output
    for(int i=0; i<(*sub)->num_test_outputs; i++){
        fprintf(fp,"%s ",(*sub)->outputs[i]);
    }
    // move to next line 
    fprintf(fp,"\n");

    // do the calculation for every input value
    //for every value of inputs
    for(int i=0; i<(*sub)->all_inputs_values_counter[0]; i++){
        //for every input
        for(int j=0; j<(*sub)->num_inputs; j++){
            (*sub)->input_values[j]=(*sub)->all_inputs_values[j][i];

        }
        //do the full calculation
        int a=doubleBufferSystem((*sub));
    
        // get the final outputs from the buffer and put them to the sumsystem
        a=findTheSubsOutputs(sub);
    
        a=findTestbenchOutputs(sub);

        //print the inputs and outputs on the output file
        for(int j=0; j<(*sub)->num_inputs; j++){
            fprintf(fp,"%d ",(*sub)->input_values[j]);
        }
        // print '|'
        fprintf(fp,"| ");

        for(int j=0; j<(*sub)->num_test_outputs; j++){
            fprintf(fp,"%d ",(*sub)->output_test_values[j]);
        }
        fprintf(fp,"\n");
    }

}

int main(){
    library* l= malloc(sizeof(library));

    int b=getLibraryFromFile("Comp_Library.txt",l);



    subsystem* s= malloc(sizeof(subsystem));
     int a=getSubSystemFromFile("dollas_netlist.txt",s,l);
   // s->inputs= malloc(sizeof(char*)*3);
   a=readTestbench("dollas_testbench.txt",&s);
//     s->input_values= (int*)malloc(sizeof(int*)*4);
    
//     s->input_values[0]=0;
//     s->input_values[1]=1;
//     s->input_values[2]=1;
//     s->num_inputs=3;

//     a=doubleBufferSystem(s);

    

//    a=findTheSubsOutputs(s);
    
   
//     s->inputs[0]="A";
//         s->inputs[1]="B";
//         s->inputs[2]="C";
//    a=findTestbenchOutputs(&s);
       // for(int i=0; i<s->num_inputs; i++){
    //    for(int j=0; j<s->all_inputs_values_counter[i]; j++){
    //     printf("%s %d\n",s->inputs[i],s->all_inputs_values[i][j]);
    //    }
    //    printf("\n");
    // }
    // for(int i=0; i<s->num_outputs; i++){
    //     printf("%s",s->mappings_out[i]);
    // }
    // for(int i=0; i<s->num_test_outputs; i++){
    //     printf("%s",s->outputs[i]);
    // }


    a=doForEveryValue(&s);
}