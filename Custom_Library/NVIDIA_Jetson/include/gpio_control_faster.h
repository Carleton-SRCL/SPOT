#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtwtypes.h"

void change_gpio_value(int gpioPin, int newValue);
void export_gpio(int gpioPin);
void unexport_gpio(int gpioPin);
void write_gpio_value(int gpioPin, int value);
void set_pin_direction(int gpioPin, int direction);

#ifdef __cplusplus
}
#endif

#endif  // GPIO_CONTROL_H
