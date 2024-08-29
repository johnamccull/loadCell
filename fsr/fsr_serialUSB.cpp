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



            if (bytesRead >= 10) {  // 4 bytes header + 3 * 2 bytes for ADC values = 10 bytes
                for (size_t i = 0; i <= bytesRead - 10; ++i) {
                    // Check for the header
                    if (response[i] == 0xAA && response[i + 1] == 0x55 && response[i + 2] == 0xAA && response[i + 3] == 0x55) {
                        // Decode the ADC values
                        uint16_t adcValues[3];
                        adcValues[0] = (response[i + 4] << 8) | response[i + 5];
                        adcValues[1] = (response[i + 6] << 8) | response[i + 7];
                        adcValues[2] = (response[i + 8] << 8) | response[i + 9];

                        // Print the ADC values
                        std::cout << "ADC Values: "
                                  << adcValues[0] << " "
                                  << adcValues[1] << " "
                                  << adcValues[2] << std::endl;

                        // Move the index forward to skip over the current data
                        i += 9;
                    }
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
