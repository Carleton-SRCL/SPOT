// io/read-file-sum.cpp - Read integers from file and print sum.
// Fred Swartz 2003-08-20

#include <iostream>
#include <iomanip>
#include <fstream>

#include "resource_headers.h"

using namespace std;

double whoAmI()
{
  double platformID = 0;
  std::ifstream whoAmI_textFile("/home/pi/ExperimentSoftware/WhoAmI.txt");
  whoAmI_textFile >> platformID;
  return platformID;

}