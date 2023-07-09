// example_rigid1.cc -*- C++ -*-
// simple rigid tracking program

/***
Copyright (c) PhaseSpace, Inc 2017

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

/* Link with library file wsock32.lib */
//#include "stdafx.h"

//!!!!!!!!!NETWORK CODE!!!!!!!!!!!!!!!
#pragma comment (lib, "wsock32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <Windows.h>
#include <time.h>
#include <chrono>
#define BUFFER_SIZE 4096
//!!!!!!!!!NETWORK CODE!!!!!!!!!!!!!!!/

#include <iostream>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <cmath>

#include "owl.hpp"

//#include <ctime>

using namespace std;

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, const char **argv)
{
	if (argc < 2) {
		// print error message if no value is provided
		std::cerr << "Error: no frequency value provided" << std::endl;
		return 1; // exit program with error code
	}


  WSADATA w;							/* Used to open windows connection */
  unsigned short port_number = 31535;	/* Port number to use */
  int a1, a2, a3, a4;					/* Components of address in xxx.xxx.xxx.xxx form */
  int client_length;					/* Length of client struct */
  int bytes_received = -1;				/* Bytes received from client */
  SOCKET sd;							/* Socket descriptor of server */
  struct sockaddr_in server;			/* Information about the server */
  struct sockaddr_in client;			/* Information about the client */
  double dataPacket[10] = { 0 };		/* Create array for data storage */
  double duration;
  const double TWO_PI = 2.0 * 3.14159265358979323846;
  const double M_PI = 3.14159265358979323846;

  /* Below is the IP address for the groundstation computer. In this version, 
  it has been set to my laptop. This will have to be changed for any new groundstation. */
  a1 = 192;
  a2 = 168;
  a3 = 1;
  a4 = 104;

  /* Open windows connection */
  if (WSAStartup(0x0101, &w) != 0)
  {
	  //fprintf(stderr, "Could not open Windows connection.\n");
	  ::exit(0);
  }

  /* Open a datagram socket */
  sd = socket(AF_INET, SOCK_DGRAM, 0);

  u_long mode = 1;  
  ioctlsocket(sd, FIONBIO, &mode); 

  if (sd == INVALID_SOCKET)
  {
	  fprintf(stderr, "Could not create socket.\n");
	  
	  WSACleanup();
	  ::exit(0);
  }

  /* Clear out server struct */
  memset((void *)&server, '\0', sizeof(struct sockaddr_in));

  /* Set family and port */
  server.sin_family = AF_INET;
  server.sin_port = htons(port_number);

  server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
  server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
  server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
  server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;

  /* Print out server information */
  printf("Server running on %u.%u.%u.%u\n", (unsigned char)server.sin_addr.S_un.S_un_b.s_b1,
	  (unsigned char)server.sin_addr.S_un.S_un_b.s_b2,
	  (unsigned char)server.sin_addr.S_un.S_un_b.s_b3,
	  (unsigned char)server.sin_addr.S_un.S_un_b.s_b4);
  printf("Use the ON/OFF Toggle in the SPOT Application to shut down cleanly. This console can be minimized.\n");
  

  /* Bind address to socket */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
  {
	  fprintf(stderr, "Could not bind name to socket.\n");
	  
	  closesocket(sd);
	  WSACleanup();
	  ::exit(0);
	  
  }

  client_length = (int)sizeof(struct sockaddr_in);

  /* Below is the IP address for the UDP client. */
  std::vector<std::string> client_addresses = { "192.168.1.110", "192.168.1.111", "192.168.1.112", "192.168.1.104"};

  // set up sockaddr_in struct for wireless computer
  client.sin_family = AF_INET;
  client.sin_port = htons(31534); // use port 1234 on wireless computer

  /* This is where the PhaseSpace code begins. */
  string address = "192.168.1.109";
  OWL::Context owl;
  OWL::Markers markers;
  OWL::Rigids rigids;

  if(owl.open(address) <= 0 || owl.initialize() <= 0) return 0;

  std::string phaseSpaceOptions;
  std::string tracker_id_RED_7_pos_string, tracker_id_RED_1_pos_string;
  std::string tracker_id_RED_3_pos_string, tracker_id_RED_5_pos_string;
  std::string tracker_id_BLACK_29_pos_string, tracker_id_BLACK_27_pos_string;
  std::string tracker_id_BLACK_25_pos_string, tracker_id_BLACK_31_pos_string;
  std::string tracker_id_BLUE_8_pos_string, tracker_id_BLUE_14_pos_string;
  std::string tracker_id_BLUE_12_pos_string, tracker_id_BLUE_10_pos_string;
  phaseSpaceOptions = "profile=all120";

  std::unordered_map<std::string, std::string> tracker_positions;

  // Default values
  tracker_positions["tracker_id_RED_5_pos_string"] = "pos=125.509767,143.875167,0";
  tracker_positions["tracker_id_RED_3_pos_string"] = "pos=125.509767,-135.624833,0";
  tracker_positions["tracker_id_RED_1_pos_string"] = "pos=-154.990233,-135.624833,0";
  tracker_positions["tracker_id_RED_7_pos_string"] = "pos=-153.490233,144.375167,0";
  tracker_positions["tracker_id_BLACK_29_pos_string"] = "pos=130.251807,141.800150,0";
  tracker_positions["tracker_id_BLACK_27_pos_string"] = "pos=130.751807,-135.699850,0";
  tracker_positions["tracker_id_BLACK_25_pos_string"] = "pos=-146.748193,-135.199850,0";
  tracker_positions["tracker_id_BLACK_31_pos_string"] = "pos=-146.748193,143.300150,0";
  tracker_positions["tracker_id_BLUE_8_pos_string"] = "pos=140.000177,152.096588,0";
  tracker_positions["tracker_id_BLUE_14_pos_string"] = "pos=140.500177,-125.403412,0";
  tracker_positions["tracker_id_BLUE_12_pos_string"] = "pos=-136.999823,-124.903412,0";
  tracker_positions["tracker_id_BLUE_10_pos_string"] = "pos=-136.999823,153.596588,0";

  // Attempt to read from file
  std::ifstream file("tracker_positions_phasespace.txt");
  if (file.is_open()) {
	  std::string line;
	  while (getline(file, line)) {
		  std::istringstream iss(line);
		  std::string id, x, y, z;
		  getline(iss, id, ',');
		  getline(iss, x, ',');
		  getline(iss, y, ',');
		  getline(iss, z);
		  tracker_positions[id] = "pos=" + x + "," + y + "," + z;
	  }
	  file.close();
  }

  // Output the tracker positions (from file if possible, otherwise defaults)
  for (const auto& pair : tracker_positions) {
	  std::cout << pair.first << " = " << pair.second << std::endl;
  }

  tracker_id_RED_5_pos_string = tracker_positions["tracker_id_RED_5_pos_string"];
  tracker_id_RED_3_pos_string = tracker_positions["tracker_id_RED_3_pos_string"];
  tracker_id_RED_1_pos_string = tracker_positions["tracker_id_RED_1_pos_string"];
  tracker_id_RED_7_pos_string = tracker_positions["tracker_id_RED_7_pos_string"];

  tracker_id_BLACK_29_pos_string = tracker_positions["tracker_id_BLACK_29_pos_string"];
  tracker_id_BLACK_27_pos_string = tracker_positions["tracker_id_BLACK_27_pos_string"];
  tracker_id_BLACK_25_pos_string = tracker_positions["tracker_id_BLACK_25_pos_string"];
  tracker_id_BLACK_31_pos_string = tracker_positions["tracker_id_BLACK_31_pos_string"];

  tracker_id_BLUE_8_pos_string = tracker_positions["tracker_id_BLUE_8_pos_string"];
  tracker_id_BLUE_14_pos_string = tracker_positions["tracker_id_BLUE_14_pos_string"];
  tracker_id_BLUE_12_pos_string = tracker_positions["tracker_id_BLUE_12_pos_string"];
  tracker_id_BLUE_10_pos_string = tracker_positions["tracker_id_BLUE_10_pos_string"];

  const std::string myoptions = phaseSpaceOptions;

  uint32_t tracker_id_RED = 0;
  owl.createTracker(tracker_id_RED, "rigid", "RED_rigid");

  /* Assign markers to the rigid body and indicate their positions
	 w.r.t the centre of mass (obtained from calibration text file) */
  owl.assignMarker(tracker_id_RED, 5, "5", tracker_id_RED_5_pos_string); // top left
  owl.assignMarker(tracker_id_RED, 3, "3", tracker_id_RED_3_pos_string); // top right
  owl.assignMarker(tracker_id_RED, 1, "1", tracker_id_RED_1_pos_string); // bottom right
  owl.assignMarker(tracker_id_RED, 7, "7", tracker_id_RED_7_pos_string); // bottom left 

  uint32_t tracker_id_BLACK = 2;
  owl.createTracker(tracker_id_BLACK, "rigid", "BLACK_rigid");

  /* Assign markers to the rigid body and indicate their positions
	 w.r.t the centre of mass (obtained from calibration text file) */
  owl.assignMarker(tracker_id_BLACK, 29, "29", tracker_id_BLACK_29_pos_string); // top left
  owl.assignMarker(tracker_id_BLACK, 27, "27", tracker_id_BLACK_27_pos_string); // top right
  owl.assignMarker(tracker_id_BLACK, 25, "25", tracker_id_BLACK_25_pos_string); // bottom right
  owl.assignMarker(tracker_id_BLACK, 31, "31", tracker_id_BLACK_31_pos_string); // bottom left

  uint32_t tracker_id_BLUE = 3;

  owl.createTracker(tracker_id_BLUE, "rigid", "BLUE_rigid");

  /* Assign markers to the rigid body and indicate their positions
	 w.r.t the centre of mass (obtained from calibration text file) */
  owl.assignMarker(tracker_id_BLUE, 8, "8", tracker_id_BLUE_8_pos_string); // top left
  owl.assignMarker(tracker_id_BLUE, 14, "14", tracker_id_BLUE_14_pos_string); // top right
  owl.assignMarker(tracker_id_BLUE, 12, "12", tracker_id_BLUE_12_pos_string); // bottom right
  owl.assignMarker(tracker_id_BLUE, 10, "10", tracker_id_BLUE_10_pos_string); // bottom left
  
  int frequency = atoi(argv[1]); // convert first argument to integer
  owl.frequency(frequency);


  // start streaming
  owl.streaming(1);

  //start = clock();
  auto t1 = Clock::now();

  // main loop
  while (owl.isOpen() && owl.property<int>("initialized"))
  {
	  const OWL::Event *event = owl.nextEvent(1000);
	  if (!event) continue;

	  if (event->type_id() == OWL::Type::ERROR)
	  {
		  break;
	  }
	  else if (event->type_id() == OWL::Type::FRAME)
	  {
		  if (event->find("rigids", rigids) > 0)
		  {
			  for (OWL::Rigids::iterator r = rigids.begin(); r != rigids.end(); r++)
			  {
				  if (r->cond > 0)
				  {
					  auto t2 = Clock::now();
					  duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
					  dataPacket[0] = duration * 0.000000001;

                      if (r->id == 0)
                      {
                          // Assume prevRawAngle0 and accumulatedAngle0 are declared and initialized elsewhere
                          static double prevRawAngle0 = 0.0;
                          static double accumulatedAngle0 = 0.0;

                          dataPacket[1] = r->pose[0];
                          dataPacket[2] = r->pose[1];

                          // Compute the raw angle
                          double rawAngle = atan2(2 * r->pose[4] * r->pose[5]
                              + 2 * r->pose[3] * r->pose[6],
                              2 * r->pose[3] * r->pose[3] - 1
                              + 2 * r->pose[4] * r->pose[4]);

                          // Compute the difference between the current and the previous raw angle
                          double angleDifference = rawAngle - prevRawAngle0;

                          // Normalize the angle difference to the interval [-π, π]
                          if (angleDifference <= -M_PI)
                              angleDifference += TWO_PI;
                          else if (angleDifference > M_PI)
                              angleDifference -= TWO_PI;

                          // Accumulate the angle difference
                          accumulatedAngle0 += angleDifference;

                          // Store the current raw angle for the next iteration
                          prevRawAngle0 = rawAngle;

                          // Store the accumulated angle in the data packet
                          dataPacket[3] = accumulatedAngle0;
                      }
                      else if (r->id == 2)
                      {
                          // Assume prevRawAngle2 and accumulatedAngle2 are declared and initialized elsewhere
                          static double prevRawAngle2 = 0.0;
                          static double accumulatedAngle2 = 0.0;

                          dataPacket[4] = r->pose[0];
                          dataPacket[5] = r->pose[1];

                          // Compute the raw angle
                          double rawAngle = atan2(2 * r->pose[4] * r->pose[5]
                              + 2 * r->pose[3] * r->pose[6],
                              2 * r->pose[3] * r->pose[3] - 1
                              + 2 * r->pose[4] * r->pose[4]);

                          // Compute the difference between the current and the previous raw angle
                          double angleDifference = rawAngle - prevRawAngle2;

                          // Normalize the angle difference to the interval [-π, π]
                          if (angleDifference <= -M_PI)
                              angleDifference += TWO_PI;
                          else if (angleDifference > M_PI)
                              angleDifference -= TWO_PI;

                          // Accumulate the angle difference
                          accumulatedAngle2 += angleDifference;

                          // Store the current raw angle for the next iteration
                          prevRawAngle2 = rawAngle;

                          // Store the accumulated angle in the data packet
                          dataPacket[6] = accumulatedAngle2;
                      }
                      else if (r->id == 3)
                      {
                          // Assume prevRawAngle3 and accumulatedAngle3 are declared and initialized elsewhere
                          static double prevRawAngle3 = 0.0;
                          static double accumulatedAngle3 = 0.0;

                          dataPacket[7] = r->pose[0];
                          dataPacket[8] = r->pose[1];

                          // Compute the raw angle
                          double rawAngle = atan2(2 * r->pose[4] * r->pose[5]
                              + 2 * r->pose[3] * r->pose[6],
                              2 * r->pose[3] * r->pose[3] - 1
                              + 2 * r->pose[4] * r->pose[4]);

                          // Compute the difference between the current and the previous raw angle
                          double angleDifference = rawAngle - prevRawAngle3;

                          // Normalize the angle difference to the interval [-π, π]
                          if (angleDifference <= -M_PI)
                              angleDifference += TWO_PI;
                          else if (angleDifference > M_PI)
                              angleDifference -= TWO_PI;

                          // Accumulate the angle difference
                          accumulatedAngle3 += angleDifference;

                          // Store the current raw angle for the next iteration
                          prevRawAngle3 = rawAngle;

                          // Store the accumulated angle in the data packet
                          dataPacket[9] = accumulatedAngle3;
                      }
				  }
			  }
		  }

		  // send dataPacket array to wireless computer
		  for (const auto& address : client_addresses) {
			  client.sin_addr.S_un.S_addr = inet_addr(address.c_str());
			  sendto(sd, (char*)dataPacket, sizeof(dataPacket), 0, (struct sockaddr*)&client, sizeof(client));
		  }	  
		  
	  }
  } // while 

  owl.done();
  owl.close();
  return 0;
}
