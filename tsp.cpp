#include <iostream>
#include<map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "airportcpp.h"

using namespace std;

#define R 6371
#define TO_RAD (3.1415926536 / 180)
double haversine(double th1, double ph1, double th2, double ph2)
{
   #pragma omp parallel
   double dx, dy, dz;
   ph1 -= ph2;
   ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;

   dz = sin(th1) - sin(th2);
   dx = cos(ph1) * cos(th1) - cos(th2);
   dy = sin(ph1) * cos(th1);
   return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

/**
 * parseRoutes - parses the routes in the route.dat file and
 * calculates the distance between the ariports
 *
 * IN: mat - Adjacency matrix that contains the
 * distance. mat[src][dest] where src is the starting airport and dest
 * is the destination.
 * IN: fileName - name of the routes file
 * IN: alist - list of all the airports that is used to find the
 * coorindates of the airport in the routes file.
 */
void parseRoutes(map<string, map<string, double> >& mat, string fileName,
		 map<string, Airport> alist) {
   char buf[256];
   char *token;
   string src, dest;

   FILE *fp = fopen(fileName.c_str(), "r");

   if (fp == NULL) {
      printf("Error opening routes file");
      exit(-1);
   }

   while(fgets(buf, sizeof(buf), fp)) {
      token = strtok(buf, ",");
      //pull 4th and 6th items for source and dest airline ID

      strtok(NULL, ",");

      //source
      src = strtok(NULL, ",");
      strtok(NULL, ",");
      dest = strtok(NULL, ",");

      // Haversine math here
      mat[src][dest] = haversine(alist[src].latitude, alist[src].longitude,
				 alist[dest].latitude, alist[dest].longitude);

      cout << "src = " + src + " dest = " + dest << " dist = " << mat[src][dest] <<"\n";
   }
   fclose(fp);
}

/**
 * parseAirport - parses the airport file
 *
 * IN: filename - name of the airport file to parse
 * IN: alist - hashmap of the airport and its airport code. Key is the
 * airport code and the airport struct is the airport data.
 *
 */
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

      // Skip airports that do not have an airport code since there
      // will be no reference in the routes file
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
      //cout << alist[airportCode].name + "\n";
      // printf("id: %s\n", (*alist)[airportCode].name);
   }

   printf ("\n\n\nnumber of airports = %d\n",cnt);
   fclose(fp);
   return cnt;
}


int main(int argc, char *argv[])
{
   std::map<string, Airport> airportList;
   map<string, map<string, double> > adjM;
   int count = parseAirports("airports.dat", airportList);


   //cout << "ariport = " + airportList["GKA"].name + "\n";


   parseRoutes(adjM, "routes.dat", airportList);

   // printf("lat = %s\n", airportList["GKA"].name);

   cout << "dist = " << adjM["SFO"]["HKG"] << "\n";
   return 0;
}
