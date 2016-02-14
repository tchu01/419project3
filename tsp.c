#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "airport.h"

void parseAirports(char *fileName) {
   FILE *fp = fopen(fileName, "r");
   char buf[256];
   char *token;
   Airport a; 

   if(fp == NULL) {
      printf("Error opening airports file");
      exit(-1);
   }

   while(fgets(buf, sizeof(buf), fp)) {
      token = strtok(buf, ",");
      while(token != NULL) {
         printf("%s\n", token);     
         token = strtok(NULL, ",");
      }
   }

   fclose(fp);
}
   



int main(int argc, char **argv) {
   
  parseAirports("airports.dat");  


}
