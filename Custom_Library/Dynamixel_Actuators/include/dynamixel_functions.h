#include "rtwtypes.h"

void initialize_dynamixel();
void terminate_dynamixel();

void dynamixel_controller(int, double, double, double, double, double, double, double, double, double, double, int, int, int, double, double, double, double, double, double, double);
//void dynamixel_controller(int, double, double, double, double, double, double, double, double, double, double, int, int, int, double, double, double, double, double, double, double, int32_T *);

void read_dynamixel_position(double*, double*, double*, double*, double*, double*, double, double, double);
void read_dynamixel_load(double*, double*, double*);
