#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include <cstdint>

// SX1509 registers and constants
#define SX1509_REG_I2C_DEVICE_ADDR  0x3E  
#define SX1509_REG_CLOCK            0x1E
#define SX1509_REG_MISC             0x1F
#define SX1509_REG_LED_DRIVER_ENABLE 0x20
#define SX1509_REG_I_ON_0           0x10  // Base register for bank A LED drivers

// Functions declarations for SX1509

// Initialization and configuration of SX1509
void writeRegister(uint8_t reg, uint8_t value);
int initSX1509();
void closeSX1509();
void setPWM(uint8_t channel, uint8_t value);
void commandPWM(float PWM1, float PWM2, float PWM3, float PWM4,
                float PWM5, float PWM6, float PWM7, float PWM8);


#endif // GPIO_CONTROL_H
