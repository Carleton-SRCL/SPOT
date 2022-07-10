#include <iostream>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <unistd.h>
#include <string>     // std::string, std::to_string

#include <math.h>       
#include <signal.h>

#include "owl.hpp"

using namespace std;

OWL::Context owl;
OWL::Markers markers;
OWL::Rigids rigids;

/* initialize_phasespace() initializes the options for the phasespace cameras
   and starts streaming data. */
double initialize_phasespace(double PS_SampleRate)
{
    /* The address is the IP address for the phasespace computer. */
    string address = "192.168.0.109";
    std::string phaseSpaceOptions;
    std::string tracker_id_RED_7_pos_string, tracker_id_RED_1_pos_string;
    std::string tracker_id_RED_3_pos_string, tracker_id_RED_5_pos_string;
    std::string tracker_id_BLACK_15_pos_string, tracker_id_BLACK_9_pos_string;
    std::string tracker_id_BLACK_11_pos_string, tracker_id_BLACK_13_pos_string;
    std::string tracker_id_BLUE_16_pos_string, tracker_id_BLUE_18_pos_string;
    std::string tracker_id_BLUE_20_pos_string, tracker_id_BLUE_22_pos_string;
    
    /* The options are sent to the phasespace computer. The most important 
       setting here is the frequency, which indicates how fast the phasespace
       sends data up to the groundstation. This frequency must be smaller then
       the samplerate of the function reading the data. If the frequency is 
       too high, the buffer will fill up and there will be a growing time delay
       in the data. */
    phaseSpaceOptions = "profile=all120 frequency=" + std::to_string(PS_SampleRate);
    
    /* The ID's indicate the location of each LED relative to the center of
       mass for the platform. */
    tracker_id_RED_5_pos_string = "pos=146.960175,124.9189470,0";
    tracker_id_RED_3_pos_string = "pos=144.960175,-154.081053,0";
    tracker_id_RED_1_pos_string = "pos=-133.039825,-153.581053,0";
    tracker_id_RED_7_pos_string = "pos=-131.539825,124.418947,0";
    
    tracker_id_BLACK_13_pos_string = "pos=125.944730,153.415965,0";
    tracker_id_BLACK_11_pos_string = "pos=124.944730,-125.084035,0";
    tracker_id_BLACK_9_pos_string = "pos=-153.555270,-124.584035,0";
    tracker_id_BLACK_15_pos_string = "pos=-151.805270,154.415965,0";
    
    tracker_id_BLUE_16_pos_string = "pos=149.893592,142.539807,0";
    tracker_id_BLUE_18_pos_string = "pos=-127.106408,144.039807,0";
    tracker_id_BLUE_20_pos_string = "pos=-127.106408,-134.460193,0";
    tracker_id_BLUE_22_pos_string = "pos=150.393592,-134.960193,0";

	const std::string myoptions = phaseSpaceOptions; 
    
    /* Open the TCP/IP port and send the options string. */
    if (owl.open(address) <= 0 || owl.initialize(myoptions) <= 0)
	{
		return 0; 
	}
    /* Create the rigid tracker for the RED satellite. The "rigid tracker"
       refers to the syntax used by phasespace, and it used when you want to 
       track a rigid body. */
    uint32_t tracker_id_RED = 0;
    owl.createTracker(tracker_id_RED, "rigid", "RED_rigid");

    /* Assign markers to the rigid body and indicate their positions
       w.r.t the centre of mass (obtained from calibration text file) */
    owl.assignMarker(tracker_id_RED, 5, "5", tracker_id_RED_5_pos_string); // top left
    owl.assignMarker(tracker_id_RED, 3, "3", tracker_id_RED_3_pos_string); // top right
    owl.assignMarker(tracker_id_RED, 1, "1", tracker_id_RED_1_pos_string); // bottom right
    owl.assignMarker(tracker_id_RED, 7, "7", tracker_id_RED_7_pos_string); // bottom left 

    uint32_t tracker_id_RED_arm = 1;
    owl.createTracker(tracker_id_RED_arm, "rigid", "RED_arm_flexible");

    owl.assignMarker(tracker_id_RED_arm, 2, "2", "pos=0,0,0"); // elbow 
    owl.assignMarker(tracker_id_RED_arm, 6, "6", "pos=0,0,0"); // wrist
    owl.assignMarker(tracker_id_RED_arm, 0, "0", "pos=0,0,0"); // end-effector

    uint32_t tracker_id_BLACK = 2;
    owl.createTracker(tracker_id_BLACK, "rigid", "BLACK_rigid");

    /* Assign markers to the rigid body and indicate their positions
       w.r.t the centre of mass (obtained from calibration text file) */
    owl.assignMarker(tracker_id_BLACK, 13, "13", tracker_id_BLACK_13_pos_string); // top left
    owl.assignMarker(tracker_id_BLACK, 11, "11", tracker_id_BLACK_11_pos_string); // top right
    owl.assignMarker(tracker_id_BLACK, 9,  "9", tracker_id_BLACK_9_pos_string); // bottom right
    owl.assignMarker(tracker_id_BLACK, 15, "15", tracker_id_BLACK_15_pos_string); // bottom left
    
    uint32_t tracker_id_BLUE = 3;
         
    owl.createTracker(tracker_id_BLUE, "rigid", "BLUE_rigid");

     /* Assign markers to the rigid body and indicate their positions
        w.r.t the centre of mass (obtained from calibration text file) */
    owl.assignMarker(tracker_id_BLUE, 16, "16", tracker_id_BLUE_16_pos_string); // top left
    owl.assignMarker(tracker_id_BLUE, 22, "22", tracker_id_BLUE_22_pos_string); // top right
    owl.assignMarker(tracker_id_BLUE, 20, "20", tracker_id_BLUE_20_pos_string); // bottom right
    owl.assignMarker(tracker_id_BLUE, 18, "18", tracker_id_BLUE_18_pos_string); // bottom left

    /* Start streaming phasespace data. Sending (1) streams data using TCP/IP,
       sending (2) streams data using UDP, and sending (3) streams data using
       UDP but broadcasts to all IP addresses. */
    owl.streaming(2);
   
}

/* stream_phasespace() is executed during each loop of the simulink diagram.
   This function checks if data is available; if there is data, it is sent 
   back to simulink. */
void stream_phasespace(double* XPOS_red, double* YPOS_red, 
        double* ATTI_red, double* XPOS_black, double* YPOS_black, double* ATTI_black,
        double* current_time, double* ElbowX, double* ElbowY, double* WristX, double* WristY,
        double* EndEffX, double* EndEffY, double* XPOS_blue, double* YPOS_blue, double* ATTI_blue)
{

        /* Initialize the "event" parameter. This parameter indicates if there is
           any data available. If there is no data, a zero is returned. */
        const OWL::Event *event = owl.nextEvent(1000);

        /* If the connection is available and the properties are initialized,
           check if there is data available. If there is data, then check which
           rigid body has been located. Then, loop through the different IDs 
           found and store the data. */
        if (owl.isOpen() && owl.property<int>("initialized"))
        {
            if (!event)
            {
                // Do not do anything! There is no good data.
            }
            else if (event->type_id() == OWL::Type::FRAME)
            {
                if (event->find("rigids", rigids) > 0)
                {
                    for (OWL::Rigids::iterator r = rigids.begin(); r != rigids.end(); r++)
                    {
                        if (r->cond > 0) 
                        {
                            if (r->id == 0)
                            {
                                *XPOS_red = r->pose[0];
                                *YPOS_red = r->pose[1];
                                *ATTI_red = atan2(2 * r->pose[4] * r->pose[5] 
                                        + 2 * r->pose[3] * r->pose[6], 
                                        2 * r->pose[3] * r->pose[3] - 1
                                        + 2 * r->pose[4] * r->pose[4]);
                                *current_time = event->time();
                            }
                            else if (r->id == 2)
                            {
                                *XPOS_black = r->pose[0];
                                *YPOS_black = r->pose[1];
                                *ATTI_black = atan2(2 * r->pose[4] * r->pose[5] 
                                        + 2 * r->pose[3] * r->pose[6], 
                                        2 * r->pose[3] * r->pose[3] - 1
                                        + 2 * r->pose[4] * r->pose[4]);
                                *current_time = event->time();
                            }   
                            else if (r->id == 3)
                            {
                                *XPOS_blue = r->pose[0];
                                *YPOS_blue = r->pose[1];
                                *ATTI_blue = atan2(2 * r->pose[4] * r->pose[5] 
                                        + 2 * r->pose[3] * r->pose[6], 
                                        2 * r->pose[3] * r->pose[3] - 1
                                        + 2 * r->pose[4] * r->pose[4]);
                                *current_time = event->time();
                            }  
                        }
                    }
                }
                if (event->find("markers", markers) > 0)
                {
                    for (OWL::Markers::iterator m = markers.begin(); m != markers.end(); m++)
                    {
                        if (m->cond > 0)
                        {
                            if (m->id == 2)
                            {
                                *ElbowX = m->x;
                                *ElbowY = m->y;
                            }
                            if (m->id == 6)
                            {
                                *WristX = m->x;
                                *WristY = m->y;
                            }
                            if (m->id == 0)
                            {
                                *EndEffX = m->x;
                                *EndEffY = m->y;
                            }
                        }
                    }
                }
            }
        }

}

/* terminate_phasespace() stops all cameras from running and closes all
   communication ports/clears all buffers */
void terminate_phasespace()
{ 
	owl.done();
	owl.close();   
}