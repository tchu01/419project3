#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "airport.h"

#define EARTH_RADIUS 6371
#define PI 3.14159265359   

/*
 * To do:
 * 1) parse routes.dat; should we do this first? because we can exclude cities that have no flights listed in the routes database
 *    essentially all we are getting from the airports.dat is the lat/long of the cities in the routes.dat file (maybe more)
 *    - commonality between routes.dat and airports.dat = SOURCE AIRPORT ID (unique OpenFlights identifier for source airport)
 *                                                      = DEST AIRPORT ID (unique OpenFlights identifier for dest airport)
 *    - ask if there are any deadend flights (that only go to a city that has no outgoing flights)
 * 
 * 2) create the adj matrix for the direct graph
 *    - when no flight between two cities... infinite weight
 *    - airports in the same city... zero weight
 *
 * 3) algorithm
 *    - choose lowest weight to any unvisited city
 *    - if no flights to a unvisited city... find another city as transition that does have flight to the unvisited city
 *
 */

double haversine(double lat1, double long1, double lat2, double long2) {
   double dLat = (PI/180.0) * (lat2 - lat1);
   double dLong = (PI/180.0) * (long2 - long1);
   double a = sin(dLat/2) * sin(dLat/2) + cos(lat1) * cos(lat2) * sin(dLong/2) * sin(dLong/2);
   double c = 2 * atan2(sqrt(a), sqrt(1-a));
   double d = EARTH_RADIUS * c;
   printf("%f\n", d);
   return d;
}

void calcDistance(Airport a[10000], int count, int adj[][10000]) {
   double lat1, lat2, long1, long2;
   for(int i = 0; i < count; i++) {
      for(int j = 0; j < count; j++) {
         if(adj[i][j] == 1) {
            lat1 = a[i].latitude;
            lat2 = a[j].latitude;
            long1 = a[i].longitude;
            long2 = a[j].longitude;  
            adj[i][j] = haversine(lat1, long1, lat2, long2);
         }
      }         
   }
   

}

void parseRoutes(int adj[][1000], char *fileName) {
   char buf[256];
   char *token;

   //To do: check to make sure its all zeroed
//   for(int i = 0; i < 10000; i++) {
//      for(int j = 0; j < 10000; j++) {
//         adj[i][j] = 0;
//      }
//   }

   FILE *fp = fopen(fileName, "r");
   
   if (fp == NULL) {
      printf("Error opening routes file");
      exit(-1);
   }

   while(fgets(buf, sizeof(buf), fp)) {
      token = strtok(buf, ",");
      //pull 4th and 6th items for source and dest airline ID

      strtok(NULL, ",");
      strtok(NULL, ",");
      
      //source
      int src = strtol(strtok(NULL, ","), (char **)NULL, 10);
      strtok(NULL, ",");
      int dest = strtol(strtok(NULL, ","), (char **)NULL, 10);
      printf("source: %d, dest: %d\n", src, dest);

      if(src - 1 >= 0 || dest - 1 >= 0) {
         adj[src - 1][dest - 1] = 1;
      }
      
   }
   fclose(fp);
}

int parseAirports(Airport **allAirport, char *fileName) {
   FILE *fp = fopen(fileName, "r");
   char buf[256];
   char *token;
   int cnt = 0;

   if(fp == NULL) {
      printf("Error opening airports file");
      exit(-1);
   }

   //issue: airport ID might not match up with index in a
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

	   (*allAirport)[cnt] = a;
	   cnt++;
      
      printf("id: %d\n", a.airportID);
   }

   printf ("\n\n\nnumber of airports = %d\n",cnt);

   for (int i = 0; i < cnt; i++) {
	   printf ("\n=========================================\n");
	   printf ("id = %d\n",(*allAirport)[i].airportID);
	   printf ("name = %s\n",(*allAirport)[i].name);
	   printf ("city = %s\n",(*allAirport)[i].city);
	   printf ("country = %s\n",(*allAirport)[i].country);
	   printf ("airport code = %s\n",(*allAirport)[i].airportCode);
	   printf ("lat = %lf\n",(*allAirport)[i].latitude);
	   printf ("long = %lf\n",(*allAirport)[i].longitude);
   }
   fclose(fp);
   return cnt;
}

int main(int argc, char **argv) {
  Airport *a = malloc(sizeof(Airport) * 10000);
  if (a == NULL) {
   printf("broke\n");
}
  int adj[1000][1000];
  printf("hi\n");
  int count = parseAirports(&a, "airports.dat");
  parseRoutes(adj, "routes1.dat");
  //calcDistance(a, count, adj);

}
