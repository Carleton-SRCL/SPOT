#include <fstream>
#include <iostream>

#include "gpio_control.h"

void change_gpio_value(int gpioPin, int newValue) {
    write_gpio_value(gpioPin, newValue);
}

void export_gpio(int gpioPin) {
    std::ofstream exportFile("/sys/class/gpio/export");
    if (exportFile.is_open()) {
        exportFile << gpioPin;
        exportFile.close();
    } else {
        // std::cerr << "Unable to export GPIO pin " << gpioPin << std::endl;
    }
}

void unexport_gpio(int gpioPin) {
    std::ofstream unexportFile("/sys/class/gpio/unexport");
    if (unexportFile.is_open()) {
        unexportFile << gpioPin;
        unexportFile.close();
    } else {
        // std::cerr << "Unable to unexport GPIO pin " << gpioPin << std::endl;
    }
}

void write_gpio_value(int gpioPin, int value) {
    std::string gpioPath = "/sys/class/gpio/gpio" + std::to_string(gpioPin) + "/value";
    std::ofstream valueFile(gpioPath);
    if (valueFile.is_open()) {
        valueFile << value;
        valueFile.close();
    } else {
        // std::cerr << "Unable to write value to GPIO pin " << gpioPin << std::endl;
    }
}

void set_pin_direction(int gpioPin, int direction) {
    std::string gpioPath = "/sys/class/gpio/gpio" + std::to_string(gpioPin) + "/direction";
    std::ofstream directionFile(gpioPath);
    if (directionFile.is_open()) {
        if (direction == 1)
            directionFile << "out";
        else
            directionFile << "in";
        
        directionFile.close();
    } else {
        // std::cerr << "Unable to set direction for GPIO pin " << gpioPin << std::endl;
    }
}
