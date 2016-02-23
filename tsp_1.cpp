#include <iostream>
#include <map>
#include <set>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "airportcpp.h"

using namespace std;

#define R 6371
#define CIRCUM 41000
#define TO_RAD (3.1415926536 / 180)

__attribute__ ((target(mic: 0))) double haversine(double th1, double ph1, double th2, double ph2)
{
   double dx, dy, dz;
   ph1 -= ph2;
   ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;

   dz = sin(th1) - sin(th2);
   dx = cos(ph1) * cos(th1) - cos(th2);
   dy = sin(ph1) * cos(th1);
   return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

void calculateDistances(double * mat, int matrixLen, string *adj2Ap,
			struct Coordinates coordinate) {
   double *tmpLat = coordinate.latitude;
   double *tmpLong = coordinate.longitude;
   double dx, dy, dz;

//note: see report (error we could not find solution to)
#pragma offload target(mic: 0)  inout(mat: length(matrixLen * matrixLen)) in(matrixLen) in(tmpLat: length(matrixLen)) in(tmpLong: length(matrixLen))
   {
#pragma omp parallel for
      for (int i = 0; i < matrixLen; i++) {
	 for (int j = 0; j < matrixLen; j++) {
	    //check to see if there is a flight
	    if (mat[i * matrixLen + j] == 1) {
	       double lat1 = tmpLat[i];
	       double long1 = tmpLong[i];

	       double lat2 = tmpLat[j];
	       double long2 = tmpLong[j];

	       long1 -= long2;
	       long1 *= TO_RAD, lat1 *= TO_RAD, lat2 *= TO_RAD;

	       dz = sin(lat1) - sin(lat2);
	       dx = cos(long1) * cos(lat1) - cos(lat2);
	       dy = sin(long1) * cos(lat1);

	       mat[i * matrixLen + j] = asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
	    }
	 }
      }
   }
}

/*
 * adj2AP - pass index to get Airport Code
 * lookup - pass Airport code to get index
 * aList - pass Airport Code to get Airport data
 * allCities - set of cities needed to visit
 */
void tsp(double **mat, int matrixLen, string *adj2Ap,
	 map<string, Airport> alist, map<string, int> lookup,
	 set<string> &allCities, struct Coordinates *coordinate) {
   FILE *fp = fopen("result.csv", "w");
   if(!fp) {
      printf("There was an error creating and writing to result.csv\n");
      exit(-1);
   } else {
      fprintf(fp, "City,Airport Code,Trip Distance(km), Total Distance(km)\n");
   }

   // Starting at SLO
   int cur = lookup["SBP"];
   allCities.erase(alist[adj2Ap[cur]].city);
   coordinate->visitedAirports.erase(adj2Ap[cur]);
   fprintf(fp, "San Luis Obispo,SBP,0,0\n");

   double distance = 0;
   double tmpDist = 99999;
   int tmpLoc = 0;

   // Condition to finish: visited all cities
   // Greedy approach: travel to closest airport that is not in same city
   while (!allCities.empty()) {
      tmpDist = 99999;
      tmpLoc = -1;
      // For loop that checks all flights from current airport
      // Checks to make sure not in same city
      for (int i = 0; i < matrixLen; i++) {
	 if (allCities.count(alist[adj2Ap[i]].city)) {
	    if (tmpDist > mat[cur][i] && mat[cur][i] > 0) {
	       //cout << "   possible AP = " << adj2Ap[i] << "\n";
	       tmpDist = mat[cur][i];
	       tmpLoc = i;
	    }
	 }
      }

      // Greedy alg has hit deadend... no airports to fly to from current
      // whose city has not been visited
      if(tmpLoc == -1) {
	 string dst;
	 // check that there are airports we have not visited
	 if (coordinate->visitedAirports.size() != 0) {
	    // go through all those airports
	    for(set<string>::iterator iter = coordinate->visitedAirports.begin(); iter != coordinate->visitedAirports.end(); iter++) {
	       // Find airport to city we have NOT visited
	       if(allCities.count(alist[*iter].city) == 1) {
		  dst = *iter;
		  break;
	       }
	    }
	    tmpLoc = lookup[dst];
	    if((alist[adj2Ap[cur]].city).compare(alist[dst].city) == 0) {
	       //cout << "   TRAVELING TO SAME CITY =========\n";
	       tmpDist = 0;
	    } else {
	       tmpDist = haversine(alist[adj2Ap[cur]].latitude, alist[adj2Ap[cur]].longitude, alist[dst].latitude, alist[dst].longitude);
	    }
	 }
      }

      //cout << "AP = " << adj2Ap[tmpLoc] << "\n";
      distance += tmpDist;
      cur = tmpLoc;
      coordinate->visitedAirports.erase(adj2Ap[tmpLoc]);
      allCities.erase(alist[adj2Ap[tmpLoc]].city);
      fprintf(fp, "%s,%s,%lf,%lf\n", alist[adj2Ap[tmpLoc]].city, adj2Ap[tmpLoc], tmpDist, distance);
   }
   //cout << "total distance: " << distance << "\n";

   // Fly back to SLO
   tmpDist = haversine(alist[adj2Ap[cur]].latitude, alist[adj2Ap[cur]].longitude, alist["SBP"].latitude, alist["SBP"].longitude);
   distance += tmpDist;
   fprintf(fp, "San Luis Obispo,SBP,%lf,%lf\n", tmpDist, distance);
   fclose(fp);
}

void populateCoordinates(struct Coordinates *coordinates, map<string, Airport> airportList, string *adjMatToAp, int matrixLen) {
   coordinates->latitude = (double *) calloc(sizeof(double), matrixLen);
   coordinates->longitude = (double *) calloc(sizeof(double), matrixLen);
   coordinates->visitCounter = (int *) calloc(sizeof(int), matrixLen);

#pragma omp parallel for
   for (int i = 0; i < matrixLen; i++) {
      coordinates->latitude[i] = airportList[adjMatToAp[i]].latitude;
      coordinates->longitude[i] = airportList[adjMatToAp[i]].longitude;
   }
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
int parseRoutes(double *mat, int matLen, string fileName, string *adj2Ap,
		map<string, Airport> alist, set<string>& cities,
		map<string, int>& lookup, struct Coordinates *coordinate) {
   char buf[256];
   char *token;
   string src, dest;
   int srcInx = 0, destInx = 0;
   int apSrc = 0, apDest = 0;
   int apCnt = 0, lkuCnt = 0;
   int totAp = 0;

   FILE *fp = fopen(fileName.c_str(), "r");

   if (fp == NULL) {
      printf("Error opening routes file");
      exit(-1);
   }

   while(fgets(buf, sizeof(buf), fp)) {
      token = strtok(buf, ",");
      strtok(NULL, ",");
      src = strtok(NULL, ",");
      strtok(NULL, ",");
      dest = strtok(NULL, ",");

      cities.insert(alist[src].city);
      cities.insert(alist[dest].city);

      // Determine if airport is on the adj matrix
      if (!lookup.count(src)) { // does not find src airport
	 lookup[src] = lkuCnt;
	 adj2Ap[apCnt++] = src;
	 apSrc = lkuCnt++;
	 totAp++;
	 (coordinate->visitedAirports).insert(src);
      }
      else {
	 apSrc = lookup[src];
      }

      if (!lookup.count(dest)) { // does not find dest airport
	 lookup[dest] = lkuCnt;
	 adj2Ap[apCnt++] = dest;
	 apDest = lkuCnt++;
	 totAp++;
	 (coordinate->visitedAirports).insert(dest);
      }
      else {
	 apDest = lookup[dest];
      }

      cout << "src = " << apSrc << "dest = " << apDest << "\n";
      mat[(apSrc * matLen) + apDest] = 1;
   }
   fclose(fp);
   return totAp;
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
      //printf("id: %s\n", (*alist)[airportCode].name);
   }
   //printf ("\nNumber of airports with airport codes inside airports.dat = %d\n",cnt);
   fclose(fp);
   return cnt;
}

void printMat (double *mat, int cnt) {
   for (int i = 0; i < cnt; i++) {
      cout <<i<<" : ";
      for (int j = 0; j < cnt; j++) {
	 cout<<mat[i * cnt + j]<< " ";
      }
      cout<<"\n\n";
   }
}

int main(int argc, char *argv[])
{
   std::map<string, Airport> airportList;
   set<string> cities;
   map<string, int> lookup;
   struct Coordinates coordinates;

   // Parse ariports and routes file
   int count = parseAirports(argv[1], airportList);

   string adjMatToAp[count];

   // Construct 2d array for adjacency matrix
   // double **adjMat = (double **)calloc(sizeof(double*), count);
   // for (int i = 0; i < count; i++) {
   //    adjMat[i] = (double *)calloc(sizeof(double), count);
   // }

   double *adjMat = (double *)calloc(sizeof(double), count * count);

   int totalApRoutes = parseRoutes(adjMat, count, argv[2], adjMatToAp,
				   airportList, cities, lookup, &coordinates);
   populateCoordinates(&coordinates, airportList, adjMatToAp, totalApRoutes);

   for (int j = 0; j < count; j++ ) {
     cout << "ariport " <<j <<" = " << adjMatToAp[j] << "\n";
   }

   calculateDistances(adjMat, totalApRoutes, adjMatToAp, coordinates);
   printMat(adjMat, count);
   //tsp(adjMat, totalApRoutes, adjMatToAp, airportList, lookup, cities, &coordinates);

   return 0;
}
