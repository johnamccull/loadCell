#include <iostream> // For std::cout
#include <string.h>
#include <unistd.h>
#include <algorithm> // For std::reverse
#include <bitset>
#include <climits>

using namespace std;

int main() {
    uint8_t CheckSum = 0x00;       // unsigned char
    uint8_t actualCheckSum = 0x6E;

    uint8_t Data[24] = {0x01, 0x6A, 0xF4, 0xC0, 0xEF, 0x7D, 0x33, 0xC0, 
                        0x49, 0x62, 0xC9, 0xC0, 0xA2, 0x5C, 0xC6, 0xBD, 0xA6, 0x19, 
                        0x8F, 0xBD, 0xAF, 0xDA, 0x69, 0x3E};

    uint16_t packagelength = 0x001B;
    std::cout << "packagelength: " << static_cast<int>(packagelength) << endl;

    for (int i = 0; i < static_cast<int>(packagelength); i++) {
        CheckSum += Data[i];
    }

    std::cout << "Calculated CheckSum: " << static_cast<int>(CheckSum) << endl;
    std::cout << "Expected CheckSum: " << static_cast<int>(actualCheckSum) << endl;

    return 0;
}
