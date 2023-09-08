#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cmath>

// SX1509 registers and constants
#define SX1509_REG_I2C_DEVICE_ADDR  0x3E  // You may need to adjust this based on your setup.
#define SX1509_REG_CLOCK            0x1E
#define SX1509_REG_MISC             0x1F
#define SX1509_REG_LED_DRIVER_ENABLE 0x20
#define SX1509_REG_I_ON_0           0x10  // Base register for bank A LED drivers

int FD_SX;

void writeRegister(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    if (write(FD_SX, data, 2) != 2) {
        std::cout << "Failed to write to I2C device" << std::endl;
    }
}

int initSX1509() {
    const char* device = "/dev/i2c-8";
    int address = SX1509_REG_I2C_DEVICE_ADDR;

    FD_SX = open(device, O_RDWR);
    if (FD_SX < 0) {
        std::cout << "Failed to open I2C device" << std::endl;
        return FD_SX;
    }

    if (ioctl(FD_SX, I2C_SLAVE, address) < 0) {
        std::cout << "Failed to select I2C device" << std::endl;
        close(FD_SX);
        return -1;
    }

    // Configure oscillator and divider
    writeRegister(SX1509_REG_CLOCK, 0x0F);  // Use 500Hz
    writeRegister(SX1509_REG_MISC, 0x07);   // Use f/128 divider

    // Enable LED driver on bank A (pins 0-7)
    writeRegister(SX1509_REG_LED_DRIVER_ENABLE, 0xFF);

    std::cout << "I2C Opened Successfully for SX1509:" << FD_SX << std::endl;

    return FD_SX;
}

void closeSX1509() {
    close(FD_SX);
}

void setPWM(uint8_t channel, uint8_t value) {
    if (channel >= 0 && channel < 8) {
        writeRegister(SX1509_REG_I_ON_0 + channel, value);
    } else {
        std::cout << "Invalid channel number for SX1509" << std::endl;
    }
}

void commandPWM(float PWM1, float PWM2, float PWM3, float PWM4,
                float PWM5, float PWM6, float PWM7, float PWM8) {
   
    float duties[8] = {PWM1, PWM2, PWM3, PWM4, PWM5, PWM6, PWM7, PWM8};

    // Convert percentages to SX1509's 0-255 range
    for (int channel = 0; channel <= 7; channel++) {
        uint8_t pwmValue = static_cast<uint8_t>((duties[channel] / 100.0) * 255.0);
        printf("The value is: %f\n", pwmValue);
        setPWM(channel, pwmValue);
    }
}
