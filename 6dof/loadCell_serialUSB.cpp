#include <string.h>
#include <unistd.h>
#include <algorithm> // For reverse
#include <bitset>
#include <climits>
#include <cstdint>
#include <array>
#include <vector>
#include <iostream>
#include <chrono>
#include <fstream> // For writing to CSV
#include "../SerialPort/SerialPort.hpp"
//#include "../matplotlib-cpp/matplotlibcpp.h" // Include matplotlibcpp

//namespace plt = matplotlibcpp;
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


void find_packet_start(SerialPort& serialPort) {
    uint8_t byte1;
    while (true) {
        serialPort.Read(&byte1, 1);
        if (byte1 == 0xAA) {
            uint8_t byte2;
            serialPort.Read(&byte2, 1);
            if (byte2 == 0x55) {
                return;
            }
        }
    }
}

std::vector<uint8_t> read_packet(SerialPort& serialPort) {
    find_packet_start(serialPort);

    uint8_t length_bytes[2];
    serialPort.Read(length_bytes, 2);
    uint16_t packet_length = (length_bytes[0] << 8) | length_bytes[1];

    uint8_t packet_num_bytes[2];
    serialPort.Read(packet_num_bytes, 2);
    uint16_t packet_num = (packet_num_bytes[0] << 8) | packet_num_bytes[1];

    std::vector<uint8_t> data_bytes(packet_length - 3);
    serialPort.Read(data_bytes.data(), packet_length - 3);

    return data_bytes;
}

std::vector<std::vector<float>> segment_into_floats(const std::vector<uint8_t>& data_bytes) {
    size_t num_floats = data_bytes.size() / 4;
    std::vector<float> floats(num_floats);

    for (size_t i = 0; i < num_floats; ++i) {
        uint32_t value = (data_bytes[i * 4 + 3] << 24) |
                         (data_bytes[i * 4 + 2] << 16) |
                         (data_bytes[i * 4 + 1] << 8) |
                         (data_bytes[i * 4]);
        floats[i] = *reinterpret_cast<float*>(&value);
    }

    std::vector<std::vector<float>> channel_buffers(6);
    for (size_t i = 0; i < num_floats; ++i) {
        channel_buffers[i % 6].push_back(floats[i]);
    }

    return channel_buffers;
}

void export_to_csv(const std::vector<std::vector<float>>& all_channel_data, const std::string& filename) {
    std::ofstream csv_file(filename);
    if (!csv_file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << '\n';
        return;
    }

    // Write header
    csv_file << "Channel 1,Channel 2,Channel 3,Channel 4,Channel 5,Channel 6\n";

    // Find the maximum number of samples in any channel
    size_t max_samples = 0;
    for (const auto& channel : all_channel_data) {
        max_samples = std::max(max_samples, channel.size());
    }

    // Write data
    for (size_t i = 0; i < max_samples; ++i) {
        for (size_t j = 0; j < all_channel_data.size(); ++j) {
            if (i < all_channel_data[j].size()) {
                csv_file << all_channel_data[j][i];
            }
            if (j < all_channel_data.size() - 1) {
                csv_file << ",";
            }
        }
        csv_file << "\n";
    }

    csv_file.close();
    std::cout << "Data successfully exported to " << filename << '\n';
}

void read_from_m8123b2(const std::string& port) {
    SerialPort serialPort(port, BaudRate::B_115200);
    serialPort.SetTimeout(-1);
    serialPort.Open();
    //serialPort.Write(cmd_stop_streaming.data(), cmd_stop_streaming.size());
    serialPort.FlushIO();
    std::cout << "Serial port " << port << " opened successfully.\n";

    std::cout << "Sending command as ASCII: ";
	for (const auto& byte : cmd_samp_rate_300) {
		std::cout << byte; // This works if the data represents printable characters
	}
	std::cout << std::endl;

    serialPort.Write(cmd_samp_rate_300.data(), cmd_samp_rate_300.size());
    std::string responseStr;
    serialPort.Read(responseStr);
	if (responseStr == "ACK+SMPR=300$OK\r\n") { // Check if the response matches the expected string
        std::cout << "Sensor responded with the correct confirmation: " << responseStr << std::endl;
    } else {
        std::cout << "Unexpected response from sensor: " << responseStr << std::endl;
    }
    usleep(100000);
    
    serialPort.Write(cmd_start_streaming.data(), cmd_start_streaming.size());

    std::vector<std::vector<float>> all_channel_data(6);

    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::seconds(2);

    try {
        while (std::chrono::steady_clock::now() < end_time) {
            std::vector<uint8_t> packet_data = read_packet(serialPort);
            std::vector<std::vector<float>> floats = segment_into_floats(packet_data);
            for (size_t i = 0; i < 6; ++i) {
                all_channel_data[i].insert(all_channel_data[i].end(), floats[i].begin(), floats[i].end());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    } catch (...) {
        std::cerr << "Unknown error occurred.\n";
    }

    // Visualize the data after reading
    /*for (size_t i = 0; i < all_channel_data.size(); ++i) {
        plt::plot(time_data, all_channel_data[i], {{"label", "Channel " + to_string(i + 1)}});
    }
    plt::xlabel("Time (s)");
    plt::ylabel("Amplitude");
    plt::legend();
    plt::show(); // Show the plot*/

    export_to_csv(all_channel_data, "output_data.csv");

    serialPort.Write(cmd_stop_streaming.data(), cmd_stop_streaming.size());
    serialPort.Close();
    std::cout << "Serial port " << port << " closed.\n";
}

int main() {
    std::string port = "/dev/ttyUSB0";
    read_from_m8123b2(port);
    return 0;
}





















