#include <string.h>
#include <unistd.h>
#include <algorithm> // For std::reverse
#include <bitset>
#include <climits>
#include <cstdint>
#include <array>
#include <vector>
#include <iostream>
#include "../SerialPort/SerialPort.hpp"

std::vector<uint8_t> cmd_samp_rate_200 = {0x41, 0x54, 0x2B, 0x53, 0x4D, 0x50, 0x52, 0x3D, 0x32, 0x30, 0x30, 0x0D, 0x0A};
std::vector<uint8_t> cmd_samp_rate_300 = {0x41, 0x54, 0x2B, 0x53, 0x4D, 0x50, 0x52, 0x3D, 0x33, 0x30, 0x30, 0x0D, 0x0A};
std::vector<uint8_t> cmd_start_streaming = {
    0x41, 0x54, 0x2B, 0x47, 0x45, 0x54, 0x53, 0x44, 0x3D, 0x28, 0x41,
    0x30, 0x31, 0x2C, 0x41, 0x30, 0x32, 0x2C, 0x41, 0x30, 0x33, 0x2C, 0x41, 0x30, 0x34, 0x2C,
    0x41, 0x30, 0x35, 0x2C, 0x41, 0x30, 0x36, 0x29, 0x3B, 0x4D, 0x3B, 0x31, 0x30, 0x0D, 0x0A
};
std::vector<uint8_t> cmd_stop_streaming = {0x41, 0x54, 0x2B, 0x47, 0x45, 0x54, 0x53, 0x44, 0x3D, 0x53, 0x54, 0x4F, 0x50, 0x0D, 0x0A};

void initializeSerialPort(mn::CppLinuxSerial::SerialPort& port, const std::string& device, unsigned long baudRate) {
    
    port.SetDevice(device);
    port.SetBaudRate(baudRate);  // Set baud rate
    port.SetTimeout(1000);  // Set timeout to 1000 ms for blocking read operations


    usleep(100000);  // Sleep for 0.1 seconds

    port.Write(cmd_stop_streaming.data(), cmd_stop_streaming.size());
    
    port.Close();
    usleep(100000);  // Sleep for 0.1 seconds

    usleep(100000);  // Sleep for 0.1 seconds

    port.FlushIO();
    std::cout << "Reset and flushed USB port.\n";

    port.Write(cmd_samp_rate_300.data(), cmd_samp_rate_300.size());
    std::cout << "Set the sampling rate to 300" << std::endl;    

    std::vector<uint8_t> response(256);  // Create a buffer for response
    size_t bytesRead = port.Read(response.data(), response.size());  // Read response into the buffer
    response.resize(bytesRead);  // Resize to actual bytes read

    // Convert response to string
    std::string responseStr(response.begin(), response.end());

    // Check if the response matches the expected string
    if (responseStr == "AT+SMPR=300\r\n") {
        std::cout << "Sensor responded with the correct confirmation: " << responseStr << std::endl;
    } else {
        std::cout << "Unexpected response from sensor: " << responseStr << std::endl;
    }

    usleep(500000);  // Sleep for 0.5 seconds

    port.Write(cmd_start_streaming.data(), cmd_start_streaming.size());
}

int main() {
    mn::CppLinuxSerial::SerialPort port;

    // Define the device and baud rate
    std::string device = "/dev/ttyUSB0";
    unsigned long baudRate = 115200;

    // Initialize the serial port
    initializeSerialPort(port, device, baudRate);

    // Add any additional code here, such as sending/receiving data

    return 0;
}
