#include "rtwtypes.h"

void initialize_dynamixel_position_control(double, double, double, double, double, double);
void initialize_dynamixel_speed_control(double, double, double);
void initialize_dynamixel_PWM_control(double);
void initialize_dynamixel_current_control(double);
void initialize_dynamixel_special_control(double, double, double, double, double, double, double);
void initialize_dynamixel_arm_gripper_control(double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double);
void command_dynamixel_position(double, double, double);
void command_dynamixel_speed(double, double, double);
void command_dynamixel_PWM(double, double, double);
void command_dynamixel_current(int, int, int);
void command_dynamixel_special(int, int, double);
void command_dynamixel_arm_gripper_position(double, double, double, double, double, double, double);
void read_dynamixel_position(double*, double*, double*);
void read_dynamixel_load(double*, double*, double*);
void terminate_dynamixel();