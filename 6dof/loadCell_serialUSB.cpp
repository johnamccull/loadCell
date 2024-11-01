#include <string.h>
#include <unistd.h>
#include <algorithm> // For reverse
#include <bitset>
#include <climits>
#include <cstdint>
#include <array>
#include <vector>
#include <iostream>
#include "../SerialPort/SerialPort.hpp"

using namespace std;
using namespace mn::CppLinuxSerial;

vector<uint8_t> cmd_samp_rate_200 = {0x41, 0x54, 0x2B, 0x53, 0x4D, 0x50, 0x52, 0x3D, 0x32, 0x30, 0x30, 0x0D, 0x0A};
vector<uint8_t> cmd_samp_rate_300 = {0x41, 0x54, 0x2B, 0x53, 0x4D, 0x50, 0x52, 0x3D, 0x33, 0x30, 0x30, 0x0D, 0x0A};
vector<uint8_t> cmd_start_streaming = {
    0x41, 0x54, 0x2B, 0x47, 0x45, 0x54, 0x53, 0x44, 0x3D, 0x28, 0x41,
    0x30, 0x31, 0x2C, 0x41, 0x30, 0x32, 0x2C, 0x41, 0x30, 0x33, 0x2C, 0x41, 0x30, 0x34, 0x2C,
    0x41, 0x30, 0x35, 0x2C, 0x41, 0x30, 0x36, 0x29, 0x3B, 0x4D, 0x3B, 0x31, 0x30, 0x0D, 0x0A
};
vector<uint8_t> cmd_stop_streaming = {0x41, 0x54, 0x2B, 0x47, 0x45, 0x54, 0x53, 0x44, 0x3D, 0x53, 0x54, 0x4F, 0x50, 0x0D, 0x0A};

void initializeSerialPort(SerialPort& port, const string& device, unsigned long baudRate) {
    
    port.SetDevice(device);
    port.SetBaudRate(baudRate);  // Set baud rate
    port.SetTimeout(1000);  // Set timeout to 1000 ms for blocking read operations
    port.Open();

    usleep(100000);  // Sleep for 0.1 seconds

    port.Write(cmd_stop_streaming.data(), cmd_stop_streaming.size());
    
    port.Close();
    usleep(100000);  // Sleep for 0.1 seconds

    port.Open();

    usleep(100000);  // Sleep for 0.1 seconds

    port.FlushIO();
    cout << "Reset and flushed USB port.\n";

    port.Write(cmd_samp_rate_300.data(), cmd_samp_rate_300.size());
    cout << "Set the sampling rate to 300" << endl;    

    std::string responseStr;
    port.Read(responseStr); 

    //this code below doesn't work for some reason but the above code does the same thing ?
    //vector<uint8_t> response(256);  // Create a buffer for response
    //size_t bytesRead = port.Read(response.data(), response.size());  // Read response into the buffer
    //response.resize(bytesRead);  // Resize to actual bytes read
    // Convert response to string
    //string responseStr(response.begin(), response.end());

    if (responseStr == "ACK+SMPR=300$OK\r\n") { // Check if the response matches the expected string
        cout << "Sensor responded with the correct confirmation: " << responseStr << endl;
    } else {
        cout << "Unexpected response from sensor: " << responseStr << endl;
    }

    usleep(500000);  // Sleep for 0.5 seconds

    port.Write(cmd_start_streaming.data(), cmd_start_streaming.size());
}

void find_packet_start(SerialPort& serial_port) {
    while (true) {
        uint8_t byte1;
        if (serial_port.Read(&byte1, 1) != 1) continue;
        if (byte1 == 0xAA) {
            uint8_t byte2;
            if (serial_port.Read(&byte2, 1) == 1 && byte2 == 0x55) {
                return;
            }
        }
    }
}

void readData(SerialPort& port) {
    vector<uint8_t> readBuffer(256);  // Buffer to hold the read data

    while (true) {
        //find_packet_start(port);

        // Read from serial port
        int bytesRead = port.Read(readBuffer.data(), readBuffer.size());

        if (bytesRead > 0) {
            cout << "Bytes received: " << bytesRead << "\nData: ";
            for (int i = 0; i < bytesRead; ++i) {
                cout << std::hex << static_cast<int>(readBuffer[i]) << " ";  // Print in hex format
            }
            cout << std::dec << endl;  // Switch back to decimal
        }

        usleep(100000);  // Adjust the read frequency as needed
    }
}

int main() {
    SerialPort port;

    // Define the device and baud rate
    string device = "/dev/ttyUSB0";
    unsigned long baudRate = 115200;

    // Initialize the serial port
    initializeSerialPort(port, device, baudRate);

    readData(port);

    // Close the serial port (not reachable in the current loop)
    port.Close();
    cout << "Serial port closed.\n";


    return 0;
}










