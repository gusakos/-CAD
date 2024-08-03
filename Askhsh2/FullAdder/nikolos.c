#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *name;
    int num_inputs;
} Subsystem;


typedef struct {
    char* id;
    char* name;
    int num_inputs;
    char** inputs;
} Component;


void read_from_file(char* filename, Subsystem fa){

}


void main() {


    Subsystem full_adder;
    read_from_file("petros.txt", full_adder);



    full_adder.num_inputs = 3;  // theoritika to leei to library




    int full_Adder_inputs = 3;




    Component** netlist;

    Component* full_adder1;
    full_adder1->id = "U33";
    full_adder1->name = "FULL_ADDER";
    full_adder1->inputs[0] = "A0";
    full_adder1->inputs[1] = "B0";
    full_adder1->inputs[2] = "Cin";
    
    Component* full_adder2;
    full_adder1->id = "U2";
    full_adder1->name = "FULL_ADDER";
    full_adder1->inputs[0] = "A1";
    full_adder1->inputs[1] = "B1";
    full_adder1->inputs[2] = "U33_Cout";

    netlist[0] = full_adder1;

    char* str;
    

    sprintf(str, "%s %s", netlist[0]->id, netlist[0]->name);



}
