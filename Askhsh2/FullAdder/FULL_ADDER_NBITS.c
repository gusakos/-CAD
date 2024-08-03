#include <stdio.h>
#include <stdlib.h>
#include <string.h>



        /*  Defining FULL_ADDER Comp*/
typedef struct {
    int num_inputs;
    
   
    char* id;
    char* type;
    char** inputs; 
    int num_outputs; 
    char** outputs;
}Full_adder;


        /*  Defining Netlist  Comp  made of FULL_ADDERS*/
typedef struct{
    int num_bits;
    Full_adder** addersm;
    char* type;
    char** inputs;

}netlist_nbits;


void freeAdder(Full_adder* adder){

    for(int i=0; i<adder->num_inputs; i++){
         free(adder->inputs[i]);
    }
    free(adder->outputs[0]);
    free(adder->outputs[1]);
    free(adder->id);
    free(adder->type);
    free(adder);
   
}
void freeNetlist(netlist_nbits* netlist){  
    
    for(int i=0; i<netlist->num_bits; i++){
        freeAdder(netlist->addersm[i]);
    }
    for (int i =0; i<(netlist->num_bits * 2 + 1); i++){
        free(netlist->inputs[i]);
    } 
    free(netlist->addersm);
    free(netlist->type);

}
 /** Creates a table to store the netlist so that we can print it to a file*/
char** createTable (netlist_nbits* netlist){
    // Create a table of strings to store the netlist
    char **netlist_table = malloc(30 * sizeof(char*));

    // Add the netlist components to the table of strings  
    for (int i=0; i<(5+2*netlist->num_bits); i++ ){

    
        netlist_table[i] = malloc(256 * sizeof(char));

    }
    snprintf(netlist_table[0],50,"COMP %s ; IN: A%d..0,B%d..0,%s \n",netlist->type, netlist->num_bits-1,netlist->num_bits-1,netlist->addersm[0]->inputs[2]);
    snprintf(netlist_table[1],50,"BEGIN %s NETLIST\n",netlist->type);

     for (int i=0; i<netlist->num_bits; i++){

        snprintf(netlist_table[i+2],50,"%s %s %s %s %s \n",netlist->addersm[i]->id,netlist->addersm[i]->type,netlist->addersm[i]->inputs[0],netlist->addersm[i]->inputs[1],netlist->addersm[i]->inputs[2]);    
    }

    for(int i=0; i<netlist->num_bits; i++){
        snprintf(netlist_table[2 + netlist->num_bits + i],50,"S%d= %s \n",i,netlist->addersm[i]->outputs[0]);
    }

    snprintf(netlist_table[2 + 2*netlist->num_bits],50,"Cout= %s \n",netlist->addersm[netlist->num_bits-1]->outputs[1]);
    snprintf(netlist_table[3+ 2*netlist->num_bits],50,"END %s NETLIST \n",netlist->type);

    return netlist_table;
}


        /*  Creating  FULL_ADDER Comp */
Full_adder* create_full_adder(char* input_A, char* input_B, char* input_C_in, char* type, int gate_number ) {

    // Creating a Full_adder instance 
    Full_adder *adder= malloc(sizeof(Full_adder));

    // Initializing adder's # of inputs,# of outputs and type                  
    adder->num_inputs = 3;
    adder->num_outputs=2;
    adder->type=strdup(type);

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

    // Filling adder's inputs,outputs and gate ID using the strings created earlier 
    adder->inputs[0] = strdup(input_A);                            
    adder->inputs[1] = strdup(input_B);
    adder->inputs[2] = strdup(input_C_in);
    adder->outputs[0]=strdup(output_u1);
    adder->outputs[1]=strdup(output_u2);
    adder->id = strdup(input_u);

    return adder;
}

        /*  Creating Netlist */
netlist_nbits*   createNetlist(int num){

    //Creating an instance of netlist 
    netlist_nbits* netlist= malloc(sizeof(netlist_nbits));

    // Initializing #of bits ,type of netlist
    netlist->num_bits= num;
    netlist->type= "FULL_ADDER_NBITS";

    // Creating space for every Full_adder
    netlist->addersm= malloc(sizeof(Full_adder*)* netlist->num_bits);
    
    // Creating space for netlist inputs (Inputs should have been readen by a file. We will fix this later)
    netlist->inputs= malloc(sizeof(char*) * (2 * netlist->num_bits + 1));

    // Filling Netlist inputs
    for(int i=0; i<netlist->num_bits; i++){

        // Building necessary strings as netlist inputs (should be readen by a file)
        char* input_A = malloc(sizeof(char) * 4);  
        char* input_B = malloc(sizeof(char) * 4);
        snprintf(input_A,4,"A%d",i);
        snprintf(input_B,4,"B%d",i);

         //Filling netlist inputs 
        netlist->inputs[i*2]= strdup(input_A);
        netlist->inputs[i * 2 + 1]= strdup(input_B);

    }
    netlist->inputs[2 * netlist->num_bits  ]= strdup("C_in");

    return netlist;
}

        /*  Used to print Netlist */
void printNetlist(netlist_nbits* netlist){
    printf("\nCOMP %s ; IN: A%d..0,B%d..0,%s \n",netlist->type, netlist->num_bits-1,netlist->num_bits-1,netlist->addersm[0]->inputs[2]);
    printf("BEGIN %s NETLIST\n",netlist->type);
    for (int i=0; i<netlist->num_bits; i++){

        printf("%s %s %s %s %s \n",netlist->addersm[i]->id,netlist->addersm[i]->type,netlist->addersm[i]->inputs[0],netlist->addersm[i]->inputs[1],netlist->addersm[i]->inputs[2]);    
    }
    for(int i=0; i<netlist->num_bits; i++){
        printf("S%d= %s \n",i,netlist->addersm[i]->outputs[0]);
    }
    printf("Cout= %s \n",netlist->addersm[netlist->num_bits-1]->outputs[1]);
    printf("END %s NETLIST \n",netlist->type);
}




int main() {

    int num;
    
       /*  Ask the # of bits  from the user*/
    printf("Please enter a  number of bits for the full adder from 1 to 8: ");
    scanf("%d", &num);

    if(num<1 || num>8){
        printf(" Only numbers of 1 to 8 are allowed");
        return 0;
    }

    //Create instance of Netlist
    netlist_nbits* netlist= createNetlist(num);

    //Create the first adder and put it into netlist
    Full_adder* adder0= create_full_adder(netlist->inputs[0], netlist->inputs[1], netlist->inputs[2 * netlist->num_bits ], "FULL_ADDER", 0);
    netlist->addersm[0]=adder0;

    // Creates every other adder and fills Nelist's list of Full_adders
    for(int i=1; i<netlist->num_bits; i++){

        netlist->addersm[i]= create_full_adder(netlist->inputs[i * 2], netlist->inputs[i * 2 +1], netlist->addersm[i-1]->outputs[1], "FULL_ADDER", i);

    }

           // Create a table of strings to store the netlist
    char **netlist_table = malloc(30 * sizeof(char*));
     netlist_table= createTable(netlist);

    // Open the output file for writing
    FILE *fp = fopen("netlist.txt", "w");
    if (fp == NULL) {
        printf("Error: Could not open file.\n");
        return 1;
    }

    // Write the netlist to the output file
    for (int i = 0; i < 4+ 2*netlist->num_bits; i++) {
        fprintf(fp, "%s", netlist_table[i]);
    }

    // Close the output file
    fclose(fp);



    // Free the memory allocated for the netlist
    for (int i = 0; i < 8; i++) {
        free(netlist_table[i]);
    }
    free(netlist_table);
    
    // Free everything
    freeNetlist(netlist);


}