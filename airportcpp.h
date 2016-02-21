
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

struct Airport {
   int airportID;
   string name;
   string city;
   string country;
   string airportCode;
   string ICAO;
   double latitude;
   double longitude;
   double altitude;
   double timeZone;
   char DST;
   string tzDB;
 };

struct Coordinates {
   double *latitude;
   double *longitude;
   int *visitCounter;
   set<string> visitedAirports;
};
