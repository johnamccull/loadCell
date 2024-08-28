#include <iostream>
#include "../SerialPort/SerialPort.hpp"
#include <string>
#include <thread> // For sleep functions
#include <chrono> // For time duration
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

            if (bytesRead > 0) {  // Only process if data was read
                response.resize(bytesRead);  // Resize to actual bytes read

                // Convert to string if the data is ASCII or print as hex values
                string responseStr(response.begin(), response.end());

                // Print the received data
                cout << "Received Data: " << responseStr << endl;

                // Alternatively, print as hex values
                // for (uint8_t byte : response) {
                //     cout << hex << static_cast<int>(byte) << " ";
                // }
                // cout << endl;
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
