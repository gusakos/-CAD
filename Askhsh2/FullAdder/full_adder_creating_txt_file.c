#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// FIND IN  ALREADY WRITTEN CODE


int main() {
  // Define the input and output ports
  char *inputs[] = {"A", "B", "C_in"};


  // Define every port used numbers and names 
  char *nand1_num = "u0";
  char *xor1_num = "u1";
  char *nand2_num = "u2";
  char *xor2_num = "u3";
  char *nand3_num = "u4";

  char *xor1 = "XOR2";
  char *xor2 = "XOR2";
  char *nand1 = "NAND2";
  char *nand2 = "NAND2";
  char *nand3 = "NAND2";




  // Create a table of strings to store the netlist
  char **netlist = malloc(10 * sizeof(char*));

  // Add the netlist components to the table of strings  
  netlist[0] = malloc(256 * sizeof(char));
  sprintf(netlist[0], "COMP FULL_ADDER    ; IN:A,B,Cin   ; OUT:S,Cout \nBEGIN FULL_ADDER NETLIST \n");

  netlist[1] = malloc(256 * sizeof(char));
  sprintf(netlist[1], "%s %s %s,%s\n", nand1_num, nand1, inputs[0], inputs[1]);

  netlist[2] = malloc(256 * sizeof(char));
  sprintf(netlist[2], "%s %s %s,%s\n", xor1_num,  xor1, inputs[0], inputs[1]);

  netlist[3] = malloc(256 * sizeof(char));
  sprintf(netlist[3], "%s %s %s,%s\n", nand2_num, nand2, xor1_num, inputs[2]);

  netlist[4] = malloc(256 * sizeof(char));
  sprintf(netlist[4], "%s %s %s,%s\n",  xor2_num, xor2, xor1_num, inputs[2]);

  netlist[5] = malloc(256 * sizeof(char));
  sprintf(netlist[5], "%s %s %s,%s\n", nand3_num, nand3, nand1_num, nand2_num);

  netlist[6] = malloc(256 * sizeof(char));
  sprintf(netlist[6], "S=%s, C_out=%s\n", xor2_num, nand3_num);

  netlist[7] = malloc(256 * sizeof(char));
  sprintf(netlist[7], "END FULL_ADDER NETLIST \n ");

  // Print the netlist
  printf("Netlist:\n");
  for (int i = 0; i < 8; i++) {
    printf("%s", netlist[i]);
  }

  // Open the output file for writing
  FILE *fp = fopen("Comp_full_adder.txt", "w");
  if (fp == NULL) {
    printf("Error: Could not open file.\n");
    return 1;
  }

  // Write the netlist to the output file
  for (int i = 0; i < 8; i++) {
    fprintf(fp, "%s", netlist[i]);
  }

  // Close the output file
  fclose(fp);

  // Free the memory allocated for the netlist
  for (int i = 0; i < 8; i++) {
    free(netlist[i]);
  }
  free(netlist);

  return 0;
}
