#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include <cstdint>

// PCA9685 registers and constants
#define PCA9685_MODE1        0x00
#define PCA9685_MODE2        0x01
#define PCA9685_PRESCALE     0xFE
#define PCA9685_LED0_ON_L    0x06 // first LED control register
#define OSC_CLOCK            25000000.0
#define FREQUENCY            10.0 // 50Hz PWM frequency

// Functions declarations

// Initialization and configuration of PCA9685
void initPCA9685();
void closePCA9685();
void setPWMFrequency();
void setPWM(uint8_t channel, uint16_t on, uint16_t off);

// Utility function to convert duty cycle to PWM value
uint16_t percentageToPWM(float percentage);
uint8_t readRegister(uint8_t reg);
void writeRegister(uint8_t reg, uint8_t value);

// Function to command PWM values
void commandPWM(float PWM1, float PWM2, float PWM3, float PWM4, 
                float PWM5, float PWM6, float PWM7, float PWM8);

#endif // GPIO_CONTROL_H
