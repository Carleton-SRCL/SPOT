// Get methods and members of PortHandlerLinux or PortHandlerWindows
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <tgmath.h> 

// Include the dynamixel headers
#include "dynamixel_sdk.h"
#include "dynamixel_functions.h"

// Define the device name, baudrate, and protocol
#define DEVICENAME                      "/dev/ttyUSB0"
#define BAUDRATE                        1
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
#define ADDR_MX_ACCELERATION_LIMIT      40
#define ADDR_MX_CURRENT_LIMIT           38
#define ADDR_MX_RETURN_DELAY            9
#define ADDR_MX_DRIVE_MODE              10
#define ADDR_MX_OPERATING_MODE          11
#define ADDR_MX_PWM_LIMIT               36
#define ADDR_MX_GOAL_PWM                100

// Define the port handler and packet handler variables 
dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);
dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

// Initialize GroupBulkWrite instance
dynamixel::GroupBulkWrite groupBulkWrite(portHandler, packetHandler);

// Initialize GroupBulkRead instance
dynamixel::GroupBulkRead groupBulkRead(portHandler, packetHandler);

void initialize_dynamixel_position_control(double P_GAIN, double I_GAIN, double D_GAIN, double MAX_POSITION,
                                           double MIN_POSITION, double VELOCITY_LIMIT)
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
        
    // Open COM port for serial communication with the actuators
   portHandler->openPort();
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Set up the motors for position control
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_P_GAIN, P_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_I_GAIN, I_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_D_GAIN, D_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_MAX_POSITION, MAX_POSITION, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_MIN_POSITION, MIN_POSITION, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_P_GAIN, P_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_I_GAIN, I_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_D_GAIN, D_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_MAX_POSITION, MAX_POSITION, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_MIN_POSITION, MIN_POSITION, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_P_GAIN, P_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_I_GAIN, I_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_D_GAIN, D_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_MAX_POSITION, MAX_POSITION, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_MIN_POSITION, MIN_POSITION, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);

}

void initialize_dynamixel_speed_control(double P_GAIN, double I_GAIN, double VELOCITY_LIMIT )
{

    // Define the transmission failure code
    int dxl_comm_result      = COMM_TX_FAIL;
    bool dxl_addparam_result = false;
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Open COM port for serial communication with the actuators
    portHandler->openPort();
    
    // Set up the motors for position control
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 1, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_P_GAIN, P_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_I_GAIN, I_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 1, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_P_GAIN, P_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_I_GAIN, I_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 1, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_P_GAIN, P_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_I_GAIN, I_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
}

void initialize_dynamixel_PWM_control(double PWM_LIMIT)
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
        
    // Open COM port for serial communication with the actuators
   portHandler->openPort();
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Set up the motors for position control
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 16, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_PWM_LIMIT, PWM_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 16, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_PWM_LIMIT, PWM_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 16, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_PWM_LIMIT, PWM_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
}

void initialize_dynamixel_special_control(double P_GAIN, double I_GAIN, double D_GAIN, double MAX_POSITION,
                                           double MIN_POSITION, double VELOCITY_LIMIT, double CURRENT_LIMIT)
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
        
    // Open COM port for serial communication with the actuators
   portHandler->openPort();
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Set up the motors for position control
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 0, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_CURRENT_LIMIT, CURRENT_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 0, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_CURRENT_LIMIT, CURRENT_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_P_GAIN, P_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_I_GAIN, I_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_D_GAIN, D_GAIN, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_MAX_POSITION, MAX_POSITION, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_MIN_POSITION, MIN_POSITION, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);

}

void initialize_dynamixel_current_control(double CURRENT_LIMIT)
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
        
    // Open COM port for serial communication with the actuators
   portHandler->openPort();
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Set up the motors for position control
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 0, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_CURRENT_LIMIT, CURRENT_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 0, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_CURRENT_LIMIT, CURRENT_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 0, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_CURRENT_LIMIT, CURRENT_LIMIT, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
}


void initialize_dynamixel_arm_gripper_control(double P_GAIN_ARM, double I_GAIN_ARM, double D_GAIN_ARM, double P_GAIN_PROX, 
                                              double I_GAIN_PROX, double D_GAIN_PROX,double P_GAIN_DIST, double I_GAIN_DIST,
                                              double D_GAIN_DIST, double MAX_POSITION_ARM, double MIN_POSITION_ARM, 
                                              double MAX_POSITION_PROX, double MIN_POSITION_PROX,double MAX_POSITION_DIST, 
                                              double MIN_POSITION_DIST, double VELOCITY_LIMIT_ARM, double VELOCITY_LIMIT_GRIP,
                                              double MAX_CURRENT_GRIP)
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
        
    // Open COM port for serial communication with the actuators
   portHandler->openPort();
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Set up the motors for position control
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_P_GAIN, P_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_I_GAIN, I_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 1, ADDR_MX_POSITION_D_GAIN, D_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_MAX_POSITION, MAX_POSITION_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_MIN_POSITION, MIN_POSITION_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 1, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 1, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_P_GAIN, P_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_I_GAIN, I_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_POSITION_D_GAIN, D_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_MAX_POSITION, MAX_POSITION_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_MIN_POSITION, MIN_POSITION_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 2, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 2, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_OPERATING_MODE, 3, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_P_GAIN, P_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_I_GAIN, I_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 3, ADDR_MX_POSITION_D_GAIN, D_GAIN_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_MAX_POSITION, MAX_POSITION_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_MIN_POSITION, MIN_POSITION_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 3, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT_ARM, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 3, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 4, ADDR_MX_OPERATING_MODE, 5, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 4, ADDR_MX_POSITION_P_GAIN, P_GAIN_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 4, ADDR_MX_POSITION_I_GAIN, I_GAIN_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 4, ADDR_MX_POSITION_D_GAIN, D_GAIN_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 4, ADDR_MX_MAX_POSITION, MAX_POSITION_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 4, ADDR_MX_MIN_POSITION, MIN_POSITION_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 4, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT_GRIP, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 4, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 5, ADDR_MX_OPERATING_MODE, 5, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 5, ADDR_MX_POSITION_P_GAIN, P_GAIN_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 5, ADDR_MX_POSITION_I_GAIN, I_GAIN_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 5, ADDR_MX_POSITION_D_GAIN, D_GAIN_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 5, ADDR_MX_MAX_POSITION, MAX_POSITION_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 5, ADDR_MX_MIN_POSITION, MIN_POSITION_PROX, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 5, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT_GRIP, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 5, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
    
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 6, ADDR_MX_OPERATING_MODE, 5, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 6, ADDR_MX_POSITION_P_GAIN, P_GAIN_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 6, ADDR_MX_POSITION_I_GAIN, I_GAIN_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 6, ADDR_MX_POSITION_D_GAIN, D_GAIN_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 6, ADDR_MX_MAX_POSITION, MAX_POSITION_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 6, ADDR_MX_MIN_POSITION, MIN_POSITION_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 6, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT_GRIP, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 6, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);

    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 7, ADDR_MX_OPERATING_MODE, 5, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 7, ADDR_MX_POSITION_P_GAIN, P_GAIN_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 7, ADDR_MX_POSITION_I_GAIN, I_GAIN_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write2ByteTxRx(portHandler, 7, ADDR_MX_POSITION_D_GAIN, D_GAIN_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 7, ADDR_MX_MAX_POSITION, MAX_POSITION_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 7, ADDR_MX_MIN_POSITION, MIN_POSITION_DIST, &dxl_error);
    dxl_comm_result   = packetHandler->write4ByteTxRx(portHandler, 7, ADDR_MX_VELOCITY_LIMIT, VELOCITY_LIMIT_GRIP, &dxl_error);
    dxl_comm_result   = packetHandler->write1ByteTxRx(portHandler, 7, ADDR_MX_TORQUE_ENABLE, 1, &dxl_error);
}

void command_dynamixel_position(double JOINT1_POS_RAD, double JOINT2_POS_RAD, double JOINT3_POS_RAD )
{
    
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
    
}

void read_dynamixel_position(double* JOINT1_POS_RAD, double* JOINT2_POS_RAD, double* JOINT3_POS_RAD )
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
    bool dxl_addparam_result = false;
    int32_t dxl1_present_position = 0;
    int32_t dxl2_present_position = 0;
    int32_t dxl3_present_position = 0;
    
    dxl_addparam_result = groupBulkRead.addParam(1, ADDR_MX_PRESENT_POSITION, 4);
    dxl_addparam_result = groupBulkRead.addParam(2, ADDR_MX_PRESENT_POSITION, 4);
    dxl_addparam_result = groupBulkRead.addParam(3, ADDR_MX_PRESENT_POSITION, 4);
    
    dxl_comm_result = groupBulkRead.txRxPacket();
    dxl1_present_position = groupBulkRead.getData(1, ADDR_MX_PRESENT_POSITION, 4);
    dxl2_present_position = groupBulkRead.getData(2, ADDR_MX_PRESENT_POSITION, 4);
    dxl3_present_position = groupBulkRead.getData(3, ADDR_MX_PRESENT_POSITION, 4);
    
    if (dxl1_present_position == 0)
    {
        *JOINT1_POS_RAD = dxl1_present_position;
    }
    else
    {
        *JOINT1_POS_RAD = 0.0015344*dxl1_present_position-3.14159;
    }
    
    if (dxl2_present_position == 0)
    {
        *JOINT2_POS_RAD = dxl2_present_position;
    }
    else
    {
        *JOINT2_POS_RAD = 0.0015344*dxl2_present_position-3.14159;
    }
    
    if (dxl3_present_position == 0)
    {
        *JOINT3_POS_RAD = dxl3_present_position;
    }
    else
    {
        *JOINT3_POS_RAD = 0.0015344*dxl3_present_position-3.14159;
    }
    groupBulkRead.clearParam();
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

void command_dynamixel_speed(double JOINT1_SPD_RAD, double JOINT2_SPD_RAD, double JOINT3_SPD_RAD )
{
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
    bool dxl_addparam_result = false;
    uint8_t param_goal_speed_1[4];
    uint8_t param_goal_speed_2[4];
    uint8_t param_goal_speed_3[4];

    double JOINT1_SPD_RPM = JOINT1_SPD_RAD*9.549296585513702;
    double JOINT2_SPD_RPM = JOINT2_SPD_RAD*9.549296585513702;
    double JOINT3_SPD_RPM = JOINT3_SPD_RAD*9.549296585513702;
    double JOINT1_SPD_BITS;
    double JOINT2_SPD_BITS;
    double JOINT3_SPD_BITS;
    
    if (JOINT1_SPD_RPM <= 0)
    {
        JOINT1_SPD_BITS = nearbyint(-8.7721*JOINT1_SPD_RPM + 1024);
    }
    if (JOINT2_SPD_RPM <= 0)
    {
        JOINT2_SPD_BITS = nearbyint(-8.7721*JOINT2_SPD_RPM + 1024);
    }
    if (JOINT3_SPD_RPM <= 0)
    {
        JOINT3_SPD_BITS = nearbyint(-8.7721*JOINT3_SPD_RPM + 1024);
    }
    
    if (JOINT1_SPD_RPM > 0)
    {
        JOINT1_SPD_BITS = nearbyint(8.7721*JOINT1_SPD_RPM);
    }
    if (JOINT2_SPD_RPM > 0)
    {
        JOINT2_SPD_BITS = nearbyint(8.7721*JOINT2_SPD_RPM);
    }
    if (JOINT3_SPD_RPM > 0)
    {
        JOINT3_SPD_BITS = nearbyint(8.7721*JOINT3_SPD_RPM);
    }
 
    
    // Allocate goal position value into byte array
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
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Send the raw (0->4096) initial position value.
    dxl_addparam_result = groupBulkWrite.addParam(1, ADDR_MX_GOAL_SPEED, 4, param_goal_speed_1);
    dxl_addparam_result = groupBulkWrite.addParam(2, ADDR_MX_GOAL_SPEED, 4, param_goal_speed_2);
    dxl_addparam_result = groupBulkWrite.addParam(3, ADDR_MX_GOAL_SPEED, 4, param_goal_speed_3);
    
    dxl_comm_result = groupBulkWrite.txPacket();
    groupBulkWrite.clearParam();
}

void command_dynamixel_PWM(double JOINT1_PWM, double JOINT2_PWM, double JOINT3_PWM )
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
    bool dxl_addparam_result = false;
    uint8_t param_goal_PWM_1[2];
    uint8_t param_goal_PWM_2[2];
    uint8_t param_goal_PWM_3[2];
    
    // Allocate goal position value into byte array
    param_goal_PWM_1[0] = DXL_LOBYTE((JOINT1_PWM));
    param_goal_PWM_1[1] = DXL_HIBYTE((JOINT1_PWM));
    
    param_goal_PWM_2[0] = DXL_LOBYTE((JOINT2_PWM));
    param_goal_PWM_2[1] = DXL_HIBYTE((JOINT2_PWM));
    
    param_goal_PWM_3[0] = DXL_LOBYTE((JOINT3_PWM));
    param_goal_PWM_3[1] = DXL_HIBYTE((JOINT3_PWM));
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Send the raw (0->4096) initial position value.
    dxl_addparam_result = groupBulkWrite.addParam(1, ADDR_MX_GOAL_PWM, 2, param_goal_PWM_1);
    dxl_addparam_result = groupBulkWrite.addParam(2, ADDR_MX_GOAL_PWM, 2, param_goal_PWM_2);
    dxl_addparam_result = groupBulkWrite.addParam(3, ADDR_MX_GOAL_PWM, 2, param_goal_PWM_3);
    
    dxl_comm_result = groupBulkWrite.txPacket();
    groupBulkWrite.clearParam();
    
}

void command_dynamixel_special(int JOINT1_BITS, int JOINT2_BITS, double JOINT3_POS_RAD )
{
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
    bool dxl_addparam_result = false;
    uint8_t param_goal_TAU_1[2];
    uint8_t param_goal_TAU_2[2];
    
    // Allocate goal position value into byte array
    param_goal_TAU_1[0] = DXL_LOBYTE((JOINT1_BITS));
    param_goal_TAU_1[1] = DXL_HIBYTE((JOINT1_BITS));
    
    param_goal_TAU_2[0] = DXL_LOBYTE((JOINT2_BITS));
    param_goal_TAU_2[1] = DXL_HIBYTE((JOINT2_BITS));
    
    // Define the transmission failure code
    uint8_t param_goal_position_3[4];
    
    // Convert the commanded position from radians to raw bits
    double JOINT3_POS_BITS  = nearbyint(651.74*JOINT3_POS_RAD + 2048);
    
    param_goal_position_3[0] = DXL_LOBYTE(DXL_LOWORD(JOINT3_POS_BITS));
    param_goal_position_3[1] = DXL_HIBYTE(DXL_LOWORD(JOINT3_POS_BITS));
    param_goal_position_3[2] = DXL_LOBYTE(DXL_HIWORD(JOINT3_POS_BITS));
    param_goal_position_3[3] = DXL_HIBYTE(DXL_HIWORD(JOINT3_POS_BITS));
    
    
    // Send the raw (0->4096) initial position value.
    dxl_addparam_result = groupBulkWrite.addParam(3, ADDR_MX_GOAL_POSITION, 4, param_goal_position_3);
    
    // Send the raw (0->4096) initial position value.
    dxl_addparam_result = groupBulkWrite.addParam(1, ADDR_MX_GOAL_CURRENT, 2, param_goal_TAU_1);
    dxl_addparam_result = groupBulkWrite.addParam(2, ADDR_MX_GOAL_CURRENT, 2, param_goal_TAU_2);
    
    dxl_comm_result = groupBulkWrite.txPacket();
    groupBulkWrite.clearParam();
    
}

void command_dynamixel_current(int JOINT1_BITS, int JOINT2_BITS, int JOINT3_BITS )
{
    // Convert torque to amps using performance curve
    //int JOINT1_BITS;
    //int JOINT2_BITS;
    //int JOINT3_BITS;
    
    //JOINT1_BITS = nearbyint(JOINT1_TAU/0.0030912);
    //JOINT2_BITS = nearbyint(JOINT2_TAU/0.0030912);
    //JOINT3_BITS = nearbyint(JOINT3_TAU/0.0030912);
    
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
    
}

void command_dynamixel_arm_gripper_position(double JOINT1_POS_RAD, double JOINT2_POS_RAD, double JOINT3_POS_RAD,
                                            double JOINT_L1_POS_RAD, double JOINT_R1_POS_RAD, double JOINT_L2_POS_RAD,
                                            double JOINT_R2_POS_RAD)
{
    
    // Define the transmission failure code
    int dxl_comm_result   = COMM_TX_FAIL;
    bool dxl_addparam_result = false;
    uint8_t param_goal_position_1[4];
    uint8_t param_goal_position_2[4];
    uint8_t param_goal_position_3[4];
    uint8_t param_goal_position_L1[4];
    uint8_t param_goal_position_R1[4];
    uint8_t param_goal_position_L2[4];
    uint8_t param_goal_position_R2[4];
    
    // Convert the commanded position from radians to raw bits
    double JOINT1_POS_BITS  = nearbyint(651.74*JOINT1_POS_RAD + 2048);
    double JOINT2_POS_BITS  = nearbyint(651.74*JOINT2_POS_RAD + 2048);
    double JOINT3_POS_BITS  = nearbyint(651.74*JOINT3_POS_RAD + 2048);
    double JOINT_L1_POS_BITS  = nearbyint(651.74*JOINT_L1_POS_RAD + 2048);
    double JOINT_R1_POS_BITS  = nearbyint(651.74*JOINT_R1_POS_RAD + 2048);
    double JOINT_L2_POS_BITS  = nearbyint(651.74*JOINT_L2_POS_RAD + 2048);
    double JOINT_R2_POS_BITS  = nearbyint(651.74*JOINT_R2_POS_RAD + 2048);
    
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
    
    param_goal_position_L1[0] = DXL_LOBYTE(DXL_LOWORD(JOINT_L1_POS_BITS));
    param_goal_position_L1[1] = DXL_HIBYTE(DXL_LOWORD(JOINT_L1_POS_BITS));
    param_goal_position_L1[2] = DXL_LOBYTE(DXL_HIWORD(JOINT_L1_POS_BITS));
    param_goal_position_L1[3] = DXL_HIBYTE(DXL_HIWORD(JOINT_L1_POS_BITS));
    
    param_goal_position_R1[0] = DXL_LOBYTE(DXL_LOWORD(JOINT_R1_POS_BITS));
    param_goal_position_R1[1] = DXL_HIBYTE(DXL_LOWORD(JOINT_R1_POS_BITS));
    param_goal_position_R1[2] = DXL_LOBYTE(DXL_HIWORD(JOINT_R1_POS_BITS));
    param_goal_position_R1[3] = DXL_HIBYTE(DXL_HIWORD(JOINT_R1_POS_BITS));
    
    param_goal_position_L2[0] = DXL_LOBYTE(DXL_LOWORD(JOINT_L2_POS_BITS));
    param_goal_position_L2[1] = DXL_HIBYTE(DXL_LOWORD(JOINT_L2_POS_BITS));
    param_goal_position_L2[2] = DXL_LOBYTE(DXL_HIWORD(JOINT_L2_POS_BITS));
    param_goal_position_L2[3] = DXL_HIBYTE(DXL_HIWORD(JOINT_L2_POS_BITS));
    
    param_goal_position_R2[0] = DXL_LOBYTE(DXL_LOWORD(JOINT_R2_POS_BITS));
    param_goal_position_R2[1] = DXL_HIBYTE(DXL_LOWORD(JOINT_R2_POS_BITS));
    param_goal_position_R2[2] = DXL_LOBYTE(DXL_HIWORD(JOINT_R2_POS_BITS));
    param_goal_position_R2[3] = DXL_HIBYTE(DXL_HIWORD(JOINT_R2_POS_BITS));
    
    // Initialize the dxl_error variable
    uint8_t dxl_error = 0;
    
    // Send the raw (0->4096) initial position value.
    dxl_addparam_result = groupBulkWrite.addParam(1, ADDR_MX_GOAL_POSITION, 4, param_goal_position_1);
    dxl_addparam_result = groupBulkWrite.addParam(2, ADDR_MX_GOAL_POSITION, 4, param_goal_position_2);
    dxl_addparam_result = groupBulkWrite.addParam(3, ADDR_MX_GOAL_POSITION, 4, param_goal_position_3);
    dxl_addparam_result = groupBulkWrite.addParam(4, ADDR_MX_GOAL_POSITION, 4, param_goal_position_L1);
    dxl_addparam_result = groupBulkWrite.addParam(5, ADDR_MX_GOAL_POSITION, 4, param_goal_position_R1);
    dxl_addparam_result = groupBulkWrite.addParam(6, ADDR_MX_GOAL_POSITION, 4, param_goal_position_L2);
    dxl_addparam_result = groupBulkWrite.addParam(7, ADDR_MX_GOAL_POSITION, 4, param_goal_position_R2);
    
    dxl_comm_result = groupBulkWrite.txPacket();
    groupBulkWrite.clearParam();
    
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
