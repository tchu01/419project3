#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "airport.h"

void parseAirports(char *fileName) {
   FILE *fp = fopen(fileName, "r");
   char buf[256];
   char *token;
   Airport *allAirport = malloc(sizeof(Airport) * 10000);
   int cnt = 0;

   if(fp == NULL) {
      printf("Error opening airports file");
      exit(-1);
   }

   while(fgets(buf, sizeof(buf), fp)) {
      token = strtok(buf, ",");

      // Must free all airport structs and the strings
      Airport a = {strtol(token, (char **)NULL, 10), strdup(strtok(NULL, ",")),
		   strdup(strtok(NULL, ",")), strdup(strtok(NULL, ",")),
		   strdup(strtok(NULL, ",")), strdup(strtok(NULL, ",")),
		   strtod(strtok(NULL, ","),(char **)NULL), // lat
		   strtod(strtok(NULL, ","), (char **)NULL), // long
		   strtol(strtok(NULL, ","), (char **)NULL, 10), // alt
		   strtol(strtok(NULL, ","), (char **)NULL, 10), // timezone
		   strtol(strtok(NULL, ","), (char **)NULL, 10),
		   strtok(NULL, ",")};

	      allAirport[cnt] = a;
	      cnt++;
   }

   printf ("\n\n\nnumber of airports = %d\n",cnt);

   for (int i = 0; i < cnt; i++) {
	   printf ("\n=========================================\n");
	   printf ("id = %d\n",allAirport[i].airportID);
	   printf ("name = %s\n",allAirport[i].name);
	   printf ("city = %s\n",allAirport[i].city);
	   printf ("country = %s\n",allAirport[i].country);
	   printf ("airport code = %s\n",allAirport[i].airportCode);
	   printf ("lat = %lf\n",allAirport[i].latitude);
	   printf ("long = %lf\n",allAirport[i].longitude);
   }


   fclose(fp);
}




int main(int argc, char **argv) {

  parseAirports("airports.dat");


}
