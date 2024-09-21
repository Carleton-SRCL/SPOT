// Get methods and members of PortHandlerLinux or PortHandlerWindows
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <tgmath.h> 

// To do the modulus operator on doubles
#include <cmath>

// Include the dynamixel headers
#include "dynamixel_sdk.h"
#include "dynamixel_functions.h"

// Define the device name, baudrate, and protocol
#define DEVICENAME                      "/dev/ttyUSB0"
#define BAUDRATE                        1000000
#define PROTOCOL_VERSION                2.0

// Control table addresses                
#define ADDR_MX_GOAL_POSITION           116
#define ADDR_MX_PRESENT_POSITION        132  
#define ADDR_MX_GOAL_SPEED              104
#define ADDR_MX_GOAL_CURRENT            102
#define ADDR_MX_PRESENT_SPEED           128
#define ADDR_MX_PRESENT_LOAD            126
#define ADDR_MX_POSITION_P_GAIN         84       
#define ADDR_MX_POSITION_I_GAIN         82
#define ADDR_MX_POSITION_D_GAIN         80
#define ADDR_MX_VELOCITY_P_GAIN         78       
#define ADDR_MX_VELOCITY_I_GAIN         76
#define ADDR_MX_TORQUE_ENABLE           64
#define ADDR_MX_MAX_POSITION            48
#define ADDR_MX_MIN_POSITION            52
#define ADDR_MX_VELOCITY_LIMIT          44
#define ADDR_MX_VELOCITY_PROFILE        112
#define ADDR_MX_CURRENT_LIMIT           38
#define ADDR_MX_RETURN_DELAY            9
#define ADDR_MX_DRIVE_MODE              10
#define ADDR_MX_OPERATING_MODE          11
#define ADDR_MX_PWM_LIMIT               36
#define ADDR_MX_GOAL_PWM                100
#define ADDR_MX_PROFILE_ACCELERATION    108

// Define the port handler and packet handler variables 
dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);
dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

// Initialize GroupBulkWrite instance
dynamixel::GroupBulkWrite groupBulkWrite(portHandler, packetHandler);

// Initialize GroupBulkRead instance
dynamixel::GroupBulkRead groupBulkRead(portHandler, packetHandler);

// Initialize Groupsyncread instance for Present Speed & Position [first 4 are speed, next 4 are position, luckily the values are right beside each other in the control table]
dynamixel::GroupSyncRead groupSyncRead(portHandler, packetHandler, ADDR_MX_PRESENT_SPEED, 8);

int run_once_flag = 0;

void initialize_dynamixel()
{
    
    // Open COM port for serial communication with the actuators
    portHandler->openPort();
   
    // Set port baudrate
    portHandler->setBaudRate(BAUDRATE);
    
}


        void dynamixel_controller(int CONTROL_MODE, double POSITION_P_GAIN, double POSITION_I_GAIN, double POSITION_D_GAIN, double MAX_POSITION,
                                           double MIN_POSITION, double MOVE_TIME, double JOINT1_POS_RAD, 
                                           double JOINT2_POS_RAD, double JOINT3_POS_RAD, double CURRENT_LIMIT, int JOINT1_BITS, int JOINT2_BITS, int JOINT3_BITS, double SPEED_P_GAIN,
                                           double SPEED_I_GAIN, double VELOCITY_LIMIT, double JOINT1_SPD_RAD, double JOINT2_SPD_RAD, double JOINT3_SPD_RAD, 
                                           double ACCELERATION_TIME)
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;

    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    if (CONTROL_MODE == 0 && run_once_flag == 0) {
        
        terminate_dynamixel();
        run_once_flag = 1;
        
    // Set the actuators settings
    }
    
    if (CONTROL_MODE == 1 && run_once_flag == 1) { // Set up motors for position control
        
        initialize_dynamixel();
        
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_DRIVE_MODE, 4, &dxl_error); // Velocity_profile yields the time required to reach goal position
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_P_GAIN, POSITION_P_GAIN, &dxl_error);
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_I_GAIN, POSITION_I_GAIN, &dxl_error);
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_D_GAIN, POSITION_D_GAIN, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_MAX_POSITION, MAX_POSITION, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_MIN_POSITION, MIN_POSITION, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_PROFILE, MOVE_TIME, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_PROFILE_ACCELERATION, 0, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
        
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_DRIVE_MODE, 4, &dxl_error); // Velocity_profile yields the time required to reach goal position
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_P_GAIN, POSITION_P_GAIN, &dxl_error);
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_I_GAIN, POSITION_I_GAIN, &dxl_error);
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_D_GAIN, POSITION_D_GAIN, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_MAX_POSITION, MAX_POSITION, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_MIN_POSITION, MIN_POSITION, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_PROFILE, MOVE_TIME, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_PROFILE_ACCELERATION, 0, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
        
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_DRIVE_MODE, 4, &dxl_error); // Velocity_profile yields the time required to reach goal position
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_P_GAIN, POSITION_P_GAIN, &dxl_error); // Manually set to 800 for the combined experiment!!!
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_I_GAIN, POSITION_I_GAIN, &dxl_error);
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_D_GAIN, POSITION_D_GAIN, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_MAX_POSITION, MAX_POSITION, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_MIN_POSITION, MIN_POSITION, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_PROFILE, MOVE_TIME, &dxl_error);
        dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_PROFILE_ACCELERATION, 0, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
        
        run_once_flag = 0;
        
    } else if (CONTROL_MODE == 2 && run_once_flag == 1) { // Set up motors for current control
        
        initialize_dynamixel();
        
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 0, &dxl_error);
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_CURRENT_LIMIT, CURRENT_LIMIT, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);

        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 0, &dxl_error);
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_CURRENT_LIMIT, CURRENT_LIMIT, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);

        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 0, &dxl_error);
        dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_CURRENT_LIMIT, CURRENT_LIMIT, &dxl_error);
        dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);        
        
        run_once_flag = 0;
        
    } else if (CONTROL_MODE == 3 && run_once_flag == 1) { // Set up motors for speed control
        
        initialize_dynamixel();
        
        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 1, &dxl_error); // Velocity mode
        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_DRIVE_MODE, 4, &dxl_error); // Acceleration_profile yields the time required to reach goal velocity
        dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_P_GAIN, SPEED_P_GAIN, &dxl_error);
        dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_I_GAIN, SPEED_I_GAIN, &dxl_error);
        dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);    
        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);    

        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 1, &dxl_error);
        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_DRIVE_MODE, 4, &dxl_error); // Acceleration_profile yields the time required to reach goal velocity
        dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_P_GAIN, SPEED_P_GAIN, &dxl_error);
        dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_I_GAIN, SPEED_I_GAIN, &dxl_error);
        dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);

        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 1, &dxl_error);
        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_DRIVE_MODE, 4, &dxl_error); // Acceleration_profile yields the time required to reach goal velocity
        dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_P_GAIN, SPEED_P_GAIN, &dxl_error);
        dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_I_GAIN, SPEED_I_GAIN, &dxl_error);
        dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
        dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error); 
        
        run_once_flag = 0;
    }
    
    // Run the controller 
    if (CONTROL_MODE == 1) { // Set up motors for position control

        // Define the transmission failure code
        int dxl_comm_result   = COMM_TX_FAIL;
        bool dxl_addparam_result = false;
        uint8_t param_goal_position_1[4];
        uint8_t param_goal_position_2[4];
        uint8_t param_goal_position_3[4];

        // Convert the commanded position from radians to raw bits
        double JOINT1_POS_BITS  = nearbyint(651.74*JOINT1_POS_RAD + 2048);
        double JOINT2_POS_BITS  = nearbyint(651.74*JOINT2_POS_RAD + 2048);
        double JOINT3_POS_BITS  = nearbyint(651.74*JOINT3_POS_RAD + 2048);

        // Allocate goal position value into byte array
        param_goal_position_1[0] = DXL_LOBYTE(DXL_LOWORD(JOINT1_POS_BITS));
        param_goal_position_1[1] = DXL_HIBYTE(DXL_LOWORD(JOINT1_POS_BITS));
        param_goal_position_1[2] = DXL_LOBYTE(DXL_HIWORD(JOINT1_POS_BITS));
        param_goal_position_1[3] = DXL_HIBYTE(DXL_HIWORD(JOINT1_POS_BITS));

        param_goal_position_2[0] = DXL_LOBYTE(DXL_LOWORD(JOINT2_POS_BITS));
        param_goal_position_2[1] = DXL_HIBYTE(DXL_LOWORD(JOINT2_POS_BITS));
        param_goal_position_2[2] = DXL_LOBYTE(DXL_HIWORD(JOINT2_POS_BITS));
        param_goal_position_2[3] = DXL_HIBYTE(DXL_HIWORD(JOINT2_POS_BITS));

        param_goal_position_3[0] = DXL_LOBYTE(DXL_LOWORD(JOINT3_POS_BITS));
        param_goal_position_3[1] = DXL_HIBYTE(DXL_LOWORD(JOINT3_POS_BITS));
        param_goal_position_3[2] = DXL_LOBYTE(DXL_HIWORD(JOINT3_POS_BITS));
        param_goal_position_3[3] = DXL_HIBYTE(DXL_HIWORD(JOINT3_POS_BITS));

        // Initialize the dxl_error variable
        uint8_t dxl_error = 0;

        // Send the raw (0->4096) initial position value.
        dxl_addparam_result = groupBulkWrite.addParam(1, ADDR_MX_GOAL_POSITION, 4, param_goal_position_1);
        dxl_addparam_result = groupBulkWrite.addParam(2, ADDR_MX_GOAL_POSITION, 4, param_goal_position_2);
        dxl_addparam_result = groupBulkWrite.addParam(3, ADDR_MX_GOAL_POSITION, 4, param_goal_position_3);

        dxl_comm_result = groupBulkWrite.txPacket();
        groupBulkWrite.clearParam();
    
    } else if (CONTROL_MODE == 2) { // Set up motors for current control
   
        // Define the transmission failure code
        int dxl_comm_result   = COMM_TX_FAIL;
        bool dxl_addparam_result = false;
        uint8_t param_goal_TAU_1[2];
        uint8_t param_goal_TAU_2[2];
        uint8_t param_goal_TAU_3[2];

        // Allocate goal position value into byte array
        param_goal_TAU_1[0] = DXL_LOBYTE((JOINT1_BITS));
        param_goal_TAU_1[1] = DXL_HIBYTE((JOINT1_BITS));

        param_goal_TAU_2[0] = DXL_LOBYTE((JOINT2_BITS));
        param_goal_TAU_2[1] = DXL_HIBYTE((JOINT2_BITS));

        param_goal_TAU_3[0] = DXL_LOBYTE((JOINT3_BITS));
        param_goal_TAU_3[1] = DXL_HIBYTE((JOINT3_BITS));

        uint8_t dxl_error = 0;

        // Send the raw (0->4096) initial position value.
        dxl_addparam_result = groupBulkWrite.addParam(1, ADDR_MX_GOAL_CURRENT, 2, param_goal_TAU_1);
        dxl_addparam_result = groupBulkWrite.addParam(2, ADDR_MX_GOAL_CURRENT, 2, param_goal_TAU_2);
        dxl_addparam_result = groupBulkWrite.addParam(3, ADDR_MX_GOAL_CURRENT, 2, param_goal_TAU_3);

        dxl_comm_result = groupBulkWrite.txPacket();
        groupBulkWrite.clearParam();   
    
    } else if (CONTROL_MODE == 3) { // Set up motors for speed control
       
       // Define the transmission failure code
        int dxl_comm_result   = COMM_TX_FAIL;
        bool dxl_addparam_result = false;
        uint8_t param_goal_speed_1[4];
        uint8_t param_goal_speed_2[4];
        uint8_t param_goal_speed_3[4];
        uint8_t acceleration_time_bits[4];

        // Initialize the dxl_error variable
        uint8_t dxl_error = 0;

        // Convert to RPM
        double JOINT1_SPD_RPM = JOINT1_SPD_RAD*9.549296585513702;
        double JOINT2_SPD_RPM = JOINT2_SPD_RAD*9.549296585513702;
        double JOINT3_SPD_RPM = JOINT3_SPD_RAD*9.549296585513702;

        // Convert to bits (max speed of 234.27 corresponds to bit 1023)
        double JOINT1_SPD_BITS;
        double JOINT2_SPD_BITS;
        double JOINT3_SPD_BITS;
        JOINT1_SPD_BITS = nearbyint(JOINT1_SPD_RPM * 4.366756307);
        JOINT2_SPD_BITS = nearbyint(JOINT2_SPD_RPM * 4.366756307);
        JOINT3_SPD_BITS = nearbyint(JOINT3_SPD_RPM * 4.366756307);

        // If any commands are negative, take their two's compliment (for the 4 byte [32 bit] number)
        if (JOINT1_SPD_BITS < 0)
        {		
            JOINT1_SPD_BITS = JOINT1_SPD_BITS + pow(2,32);
        }
        if (JOINT2_SPD_BITS < 0)
        {		
            JOINT2_SPD_BITS = JOINT2_SPD_BITS + pow(2,32);
        }
        if (JOINT3_SPD_BITS < 0)
        {		
            JOINT3_SPD_BITS = JOINT3_SPD_BITS + pow(2,32);
        }

        // Allocate these into the byte array
        param_goal_speed_1[0] = DXL_LOBYTE(DXL_LOWORD(JOINT1_SPD_BITS));
        param_goal_speed_1[1] = DXL_HIBYTE(DXL_LOWORD(JOINT1_SPD_BITS));
        param_goal_speed_1[2] = DXL_LOBYTE(DXL_HIWORD(JOINT1_SPD_BITS));
        param_goal_speed_1[3] = DXL_HIBYTE(DXL_HIWORD(JOINT1_SPD_BITS));

        param_goal_speed_2[0] = DXL_LOBYTE(DXL_LOWORD(JOINT2_SPD_BITS));
        param_goal_speed_2[1] = DXL_HIBYTE(DXL_LOWORD(JOINT2_SPD_BITS));
        param_goal_speed_2[2] = DXL_LOBYTE(DXL_HIWORD(JOINT2_SPD_BITS));
        param_goal_speed_2[3] = DXL_HIBYTE(DXL_HIWORD(JOINT2_SPD_BITS));

        param_goal_speed_3[0] = DXL_LOBYTE(DXL_LOWORD(JOINT3_SPD_BITS));
        param_goal_speed_3[1] = DXL_HIBYTE(DXL_LOWORD(JOINT3_SPD_BITS));
        param_goal_speed_3[2] = DXL_LOBYTE(DXL_HIWORD(JOINT3_SPD_BITS));
        param_goal_speed_3[3] = DXL_HIBYTE(DXL_HIWORD(JOINT3_SPD_BITS));

        acceleration_time_bits[0] = DXL_LOBYTE(DXL_LOWORD(nearbyint(ACCELERATION_TIME)));
        acceleration_time_bits[1] = DXL_HIBYTE(DXL_LOWORD(nearbyint(ACCELERATION_TIME)));
        acceleration_time_bits[2] = DXL_LOBYTE(DXL_HIWORD(nearbyint(ACCELERATION_TIME)));
        acceleration_time_bits[3] = DXL_HIBYTE(DXL_HIWORD(nearbyint(ACCELERATION_TIME)));

        // Send the goal velocity command
        dxl_addparam_result = groupBulkWrite.addParam(1, ADDR_MX_GOAL_SPEED, 4, param_goal_speed_1);
        dxl_addparam_result = groupBulkWrite.addParam(1, ADDR_MX_PROFILE_ACCELERATION, 4, acceleration_time_bits); // Acceleration time in [ms] required to reach goal velocity
        dxl_addparam_result = groupBulkWrite.addParam(2, ADDR_MX_GOAL_SPEED, 4, param_goal_speed_2);
        dxl_addparam_result = groupBulkWrite.addParam(2, ADDR_MX_PROFILE_ACCELERATION, 4, acceleration_time_bits); // Acceleration time in [ms] required to reach goal velocity
        dxl_addparam_result = groupBulkWrite.addParam(3, ADDR_MX_GOAL_SPEED, 4, param_goal_speed_3);
        dxl_addparam_result = groupBulkWrite.addParam(3, ADDR_MX_PROFILE_ACCELERATION, 4, acceleration_time_bits); // Acceleration time in [ms] required to reach goal velocity

        // Clean up
        dxl_comm_result = groupBulkWrite.txPacket();
        groupBulkWrite.clearParam(); 
        
    }
    
}


void read_dynamixel_position(double* JOINT1_POS_RAD, double* JOINT2_POS_RAD, 
                            double* JOINT3_POS_RAD, double* JOINT1_SPEED_RAD, double* JOINT2_SPEED_RAD,
                            double* JOINT3_SPEED_RAD, double JOINT1_POS_PREV, double JOINT2_POS_PREV, 
                            double JOINT3_POS_PREV)
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
    bool dxl_addparam_result = false;
    bool dxl_getdata_result = false; 
    bool dx2_getdata_result = false; 
    bool dx3_getdata_result = false; 

	// Syncread present position and velocity
	dxl_comm_result = groupSyncRead.txRxPacket();
	
    int32_t dxl1_present_position = 0;
    int32_t dxl2_present_position = 0;
    int32_t dxl3_present_position = 0;
    int32_t dxl1_present_speed = 0;
    int32_t dxl2_present_speed = 0;
    int32_t dxl3_present_speed = 0;

    // Add motors to the groupSyncRead (for read_dynamixel_position())
    dxl_addparam_result = groupSyncRead.addParam(1);
    dxl_addparam_result = groupSyncRead.addParam(2);
    dxl_addparam_result = groupSyncRead.addParam(3);

    // Check if groupsyncread data of Dynamixel#1 is available
    dxl_getdata_result = groupSyncRead.isAvailable(1, ADDR_MX_PRESENT_POSITION, 4);
    if (dxl_getdata_result == false)
    {
        *JOINT1_POS_RAD = JOINT1_POS_PREV;
    } else {
        dxl1_present_position = groupSyncRead.getData(1, ADDR_MX_PRESENT_POSITION, 4);
        double joint1_wrapped = std::fmod(0.001534363 * dxl1_present_position, 6.283185307179586); // Converting bits to rads, making 0 rad be when the arm joint is extended, and wrapping to [-pi,pi)
        if (joint1_wrapped < 0)
            joint1_wrapped += 6.283185307179586;
        *JOINT1_POS_RAD = joint1_wrapped - 3.14159;
    }

    // Check if groupsyncread data of Dynamixel#2 is available
    dx2_getdata_result = groupSyncRead.isAvailable(2, ADDR_MX_PRESENT_POSITION, 4);
    if (dx2_getdata_result == false)
    {
        *JOINT2_POS_RAD = JOINT2_POS_PREV;
    } else {
        dxl2_present_position = groupSyncRead.getData(2, ADDR_MX_PRESENT_POSITION, 4);
        double joint2_wrapped = std::fmod(0.001534363 * dxl2_present_position, 6.283185307179586); // Converting bits to rads, making 0 rad be when the arm joint is extended, and wrapping to [-pi,pi)
        if (joint2_wrapped < 0)
            joint2_wrapped += 6.283185307179586;
        *JOINT2_POS_RAD = joint2_wrapped - 3.14159;
    }

    // Check if groupsyncread data of Dynamixel#3 is available
    dx3_getdata_result = groupSyncRead.isAvailable(3, ADDR_MX_PRESENT_POSITION, 4);
    if (dx3_getdata_result == false)
    {
        *JOINT3_POS_RAD = JOINT3_POS_PREV;
    } else {
        dxl3_present_position = groupSyncRead.getData(3, ADDR_MX_PRESENT_POSITION, 4);
        double joint3_wrapped = std::fmod(0.001534363 * dxl3_present_position, 6.283185307179586); // Converting bits to rads, making 0 rad be when the arm joint is extended, and wrapping to [-pi,pi)
        if (joint3_wrapped < 0)
            joint3_wrapped += 6.283185307179586;
        *JOINT3_POS_RAD = joint3_wrapped - 3.14159;
    }

    // Get present velocity values
    dxl1_present_speed = groupSyncRead.getData(1, ADDR_MX_PRESENT_SPEED, 4);
    dxl2_present_speed = groupSyncRead.getData(2, ADDR_MX_PRESENT_SPEED, 4);
    dxl3_present_speed = groupSyncRead.getData(3, ADDR_MX_PRESENT_SPEED, 4);
    *JOINT1_SPEED_RAD = 0.023981131017500 * dxl1_present_speed; // Converting bits to rad/s
    *JOINT2_SPEED_RAD = 0.023981131017500 * dxl2_present_speed; // Converting bits to rad/s
    *JOINT3_SPEED_RAD = 0.023981131017500 * dxl3_present_speed; // Converting bits to rad/s

}

void read_dynamixel_load(double* JOINT1_LOAD, double* JOINT2_LOAD, double* JOINT3_LOAD )
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
    bool dxl_addparam_result = false;
    
    dxl_addparam_result = groupBulkRead.addParam(1, ADDR_MX_PRESENT_LOAD, 2);
    dxl_addparam_result = groupBulkRead.addParam(2, ADDR_MX_PRESENT_LOAD, 2);
    dxl_addparam_result = groupBulkRead.addParam(3, ADDR_MX_PRESENT_LOAD, 2);
    
    dxl_comm_result = groupBulkRead.txRxPacket();
    *JOINT1_LOAD = groupBulkRead.getData(1, ADDR_MX_PRESENT_LOAD, 2);
    *JOINT2_LOAD = groupBulkRead.getData(2, ADDR_MX_PRESENT_LOAD, 2);
    *JOINT3_LOAD = groupBulkRead.getData(3, ADDR_MX_PRESENT_LOAD, 2);
    
    groupBulkRead.clearParam();
}

void terminate_dynamixel()
{
      // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 0, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 0, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 0, &dxl_error);
    
    // Open COM port for serial communication with the actuators
    portHandler->closePort();

}
