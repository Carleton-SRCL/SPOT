#include <iostream>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <unistd.h>

#include <math.h>       
#include <signal.h>

#include "owl.hpp"

using namespace std;

OWL::Context owl;
OWL::Markers markers;
OWL::Rigids rigids;

/* initialize_phasespace() initializes the options for the phasespace cameras
   and starts streaming data. */
double initialize_phasespace(double platformSelection, double sampleTime)
{
    /* The address is the IP address for the phasespace computer. */
    string address = "192.168.1.109";
    std::string phaseSpaceOptions;
    std::string tracker_id_RED_7_pos_string, tracker_id_RED_1_pos_string;
    std::string tracker_id_RED_3_pos_string, tracker_id_RED_5_pos_string;
    std::string tracker_id_BLACK_29_pos_string, tracker_id_BLACK_27_pos_string;
    std::string tracker_id_BLACK_25_pos_string, tracker_id_BLACK_31_pos_string;
    std::string tracker_id_BLUE_8_pos_string, tracker_id_BLUE_14_pos_string;
    std::string tracker_id_BLUE_12_pos_string, tracker_id_BLUE_10_pos_string;
    phaseSpaceOptions = "profile=default frequency=100";

    std::unordered_map<std::string, std::string> tracker_positions;

    /* The ID's indicate the location of each LED relative to the center of
       mass for the platform. */
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

	const std::string myoptions = phaseSpaceOptions; 
    
    if (owl.open(address) <= 0 || owl.initialize() <= 0) return 0;
    
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
    owl.assignMarker(tracker_id_BLACK, 13, "5", tracker_id_BLACK_29_pos_string); // top left
    owl.assignMarker(tracker_id_BLACK, 11, "3", tracker_id_BLACK_27_pos_string); // top right
    owl.assignMarker(tracker_id_BLACK, 9, "1", tracker_id_BLACK_25_pos_string); // bottom right
    owl.assignMarker(tracker_id_BLACK, 15, "7", tracker_id_BLACK_31_pos_string); // bottom left
    
    uint32_t tracker_id_BLUE = 3;
    
    owl.createTracker(tracker_id_BLUE, "rigid", "BLUE_rigid");
    
    /* Assign markers to the rigid body and indicate their positions
     w.r.t the centre of mass (obtained from calibration text file) */
    owl.assignMarker(tracker_id_BLUE, 8, "8", tracker_id_BLUE_8_pos_string); // top left
    owl.assignMarker(tracker_id_BLUE, 14, "14", tracker_id_BLUE_14_pos_string); // top right
    owl.assignMarker(tracker_id_BLUE, 12, "12", tracker_id_BLUE_12_pos_string); // bottom right
    owl.assignMarker(tracker_id_BLUE, 10, "10", tracker_id_BLUE_10_pos_string); // bottom left
                
    // start streaming
    owl.streaming(1);
   
}

/* stream_phasespace() is executed during each loop of the simulink diagram.
   This function checks if data is available; if there is data, it is sent 
   back to simulink. */
void stream_phasespace(double* XPOS_red, double* YPOS_red, 
        double* ATTI_red, double* XPOS_black, double* YPOS_black, double* ATTI_black,
        double* ELBOX_red, double* ELBOY_red, double* WRISX_red, double* WRISY_red, 
        double* ENDEX_red, double* ENDEY_red, double platformSelection)
{
    if (platformSelection == 1)
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
                                *XPOS_black  = event->time();
                                *YPOS_black  = 0;
                                *ATTI_black  = 0;
                                *ELBOX_red   = 0;
                                *WRISX_red   = 0;
                                *ENDEX_red   = 0;
                                *ELBOY_red   = 0;
                                *WRISY_red   = 0;
                                *ENDEY_red   = 0;
                                //*current_time = event->time();
                            }
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