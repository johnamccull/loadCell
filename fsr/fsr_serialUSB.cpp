#include <iostream>
#include "../SerialPort/SerialPort.hpp"
#include <string>
#include <thread> // For sleep functions
#include <chrono> // For time duration
#include <iomanip>
using namespace std;
using namespace mn::CppLinuxSerial;

int main() {
    SerialPort serialPort;

    // Configure the serial port
    string device = "/dev/ttyACM0";  // Adjust port as needed
    unsigned long baudRate = 9600;
    serialPort.SetDevice(device);
    serialPort.SetBaudRate(baudRate);
    serialPort.SetTimeout(0); 
    serialPort.Open();

    try {
        while (true) {
            std::vector<uint8_t> response(256);  // Create a buffer for response
            size_t bytesRead = serialPort.Read(response.data(), response.size());  // Read response into the buffer
            response.resize(bytesRead);  // Resize to actual bytes read

            

            if (bytesRead >= 7) {  // Ensure we have enough bytes to read header and values
                // Extract the frame header
                uint32_t header = (response[0] << 24) | (response[1] << 16) | (response[2] << 8) | response[3];

                if (header == 0xAA55AA55) {  // Verify the header
                    // Decode the ADC values
                    uint16_t adcValues[3];
                    
                    adcValues[0] = (response[4] << 8) | response[5];
                    adcValues[1] = (response[6] << 8) | response[7];
                    adcValues[2] = (response[8] << 8) | response[9];
                    
                    // Print the ADC values
                    std::cout << "ADC Values: " 
                              << adcValues[0] << " " 
                              << adcValues[1] << " " 
                              << adcValues[2] << std::endl;
                } else {
                    std::cerr << "Invalid header: 0x" 
                              << std::hex << std::setw(8) << std::setfill('0') << header 
                              << std::dec << std::endl;
                }
            }

            // Sleep to prevent overwhelming the serial port with requests
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    serialPort.Close();
    return 0;
}
