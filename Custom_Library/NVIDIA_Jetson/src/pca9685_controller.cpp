#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cmath>  // For floor function

// PCA9685 registers and constants
#define PCA9685_MODE1        0x00
#define PCA9685_MODE2        0x01
#define PCA9685_PRESCALE     0xFE
#define PCA9685_LED0_ON_L    0x06
#define OSC_CLOCK            25000000.0
#define FREQUENCY            24.0

// Define some globals
int FD_PCA;

uint16_t percentageToPWM(float percentage) {

    return static_cast<uint16_t>((percentage / 100.0) * 4095.0);

}

uint8_t readRegister(uint8_t reg) {

    if (write(FD_PCA, &reg, 1) != 1) {
        std::cout << "Failed to write to I2C device" << std::endl;
        //exit(1);
    }

    uint8_t value;
    if (read(FD_PCA, &value, 1) != 1) {
        std::cout <<  "Failed to read from I2C device" << std::endl;
        //exit(1);
    }
    return value;

}

void writeRegister(uint8_t reg, uint8_t value) {

    uint8_t data[2] = {reg, value};
    if (write(FD_PCA, data, 2) != 2) {
        std::cout << "Failed to write to I2C device" << std::endl;
        //exit(1);
    }

}

void setPWMFrequency() {

    // Calculate and set prescale value for desired frequency
    float prescale_val = OSC_CLOCK / (4096 * FREQUENCY) - 1;
    uint8_t prescale = round(prescale_val); 
    writeRegister(PCA9685_MODE1, 0x10);   // Sleep mode
    writeRegister(PCA9685_PRESCALE, prescale);
    writeRegister(PCA9685_MODE1, 0x00);   // Wake up from sleep
    usleep(5000);                             // Wait for oscillator to stabilize
    writeRegister(PCA9685_MODE1, 0x80);   // Restart

    // Read back prescale value from the device
    //uint8_t read_prescale = readRegister(PCA9685_PRESCALE);
    //std::cout << "Expected Prescale Value: " << std::endl;
    //std::cout << "Read Prescale Value: " << std::endl;

}

int initPCA9685() {

    const char* device = "/dev/i2c-8";
    int address = 0x40;

    FD_PCA = open(device, O_RDWR);
    if (FD_PCA < 0) {
        std::cout << "Failed to open I2C device" << std::endl;
        //exit(1);
    }

    if (ioctl(FD_PCA, I2C_SLAVE, address) < 0) {
        std::cout << "Failed to select I2C device" << std::endl;
        close(FD_PCA);
        //exit(1);
    }

    // Reset PCA9685
    writeRegister(PCA9685_MODE1, 0x80);   // Restart

    std::cout << "I2C Opened Successfully:" << FD_PCA << std::endl;

    setPWMFrequency();

    return FD_PCA;

}

void closePCA9685() {

    close(FD_PCA);

}

void setPWM(uint8_t channel, uint16_t on, uint16_t off) {

    writeRegister(PCA9685_LED0_ON_L + 4 * channel, on & 0xFF);
    writeRegister(PCA9685_LED0_ON_L + 4 * channel + 1, on >> 8);
    writeRegister(PCA9685_LED0_ON_L + 4 * channel + 2, off & 0xFF);
    writeRegister(PCA9685_LED0_ON_L + 4 * channel + 3, off >> 8);

}

void commandPWM(float PWM1,float PWM2,float PWM3,float PWM4,float PWM5,float PWM6,float PWM7,float PWM8) {
   
    // Store PWM commands into array
    float duties[8] = {PWM1,PWM2,PWM3,PWM4,PWM5,PWM6,PWM7,PWM8};

    // Set PWM values for channels 1 to 8
    for (int channel = 0; channel <= 7; channel++) {

        uint16_t pwmValue = percentageToPWM(duties[channel]); 

        setPWM(channel, 0, pwmValue);

        // Read back the ON and OFF values for each channel to verify
        //uint8_t onLow = readRegister(PCA9685_LED0_ON_L + 4 * channel);
        //uint8_t onHigh = readRegister(PCA9685_LED0_ON_L + 4 * channel + 1);
        //uint16_t onValue = (onHigh << 8) | onLow;

        //uint8_t offLow = readRegister(PCA9685_LED0_ON_L + 4 * channel + 2);
        //uint8_t offHigh = readRegister(PCA9685_LED0_ON_L + 4 * channel + 3);
        //uint16_t offValue = (offHigh << 8) | offLow;

        //std::cout << "Channel " << channel << ": ON Value = " << onValue << ", OFF Value = " << offValue << std::endl;
    }
    

}
