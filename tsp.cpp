#include <iostream>
#include<map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "airportcpp.h"

using namespace std;



int parseAirports(string fileName, map<string, Airport>& alist) {
   FILE *fp = fopen(fileName.c_str(), "r");
   char buf[256];
   char *token;
   int id = 0;
   char DST;
   string name, city, country, airportCode, ICAO, tzDB;
   double latit = 0, longit = 0, altit = 0, timezone = 0;
   int cnt = 0;

   if(fp == NULL) {
      printf("Error opening airports file");
      exit(-1);
   }

   //issue: airport ID might not match up with index in a
   while(fgets(buf, sizeof(buf), fp)) {
      id = strtol(strtok(buf, ",\""), (char **)NULL, 10);
      name = strdup(strtok(NULL, ",\""));
      city = strdup(strtok(NULL, ",\""));
      country = strdup(strtok(NULL, ",\""));
      airportCode = strdup(strtok(NULL, ","));

      if (!airportCode.compare("\"\"")) { // No airport code
	 continue;
      }
      else {
	 airportCode = airportCode.substr(1, airportCode.length()-2);
	 //cout << "new code = " + airportCode + "\n";
      }

      ICAO = strdup(strtok(NULL, ","));
      latit = strtod(strtok(NULL, ","),(char **)NULL); // lat
      longit = strtod(strtok(NULL, ","), (char **)NULL); // long
      altit = strtod(strtok(NULL, ","), (char **)NULL); // alt
      timezone = strtod(strtok(NULL, ","), (char **)NULL); // timezone
      DST = *(strtok(NULL, ","));
      tzDB = strdup(strtok(NULL, ","));


      Airport a = {id, name, city, country, airportCode, ICAO, latit, longit,
		   altit, timezone, DST, tzDB};


      alist[airportCode] = a;
      cnt++;
      cout << alist[airportCode].name + "\n";
      // printf("id: %s\n", (*alist)[airportCode].name);
   }

   printf ("\n\n\nnumber of airports = %d\n",cnt);
   fclose(fp);
   return cnt;
}


int main(int argc, char *argv[])
{
   std::map<string, Airport> airportList;


   int count = parseAirports("airports.dat", airportList);

   cout << "ariport = " + airportList["GKA"].name + "\n";

   // printf("lat = %s\n", airportList["GKA"].name);

   return 0;
}
