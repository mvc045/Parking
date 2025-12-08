//
//  SerialPort.hpp
//  Parking
//
//  Created by Михаил Конюхов on 01.12.2025.
//

#ifndef SerialPort_hpp
#define SerialPort_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <mutex>

using namespace std;

// Драйвер для шлагбаума
class SerialPort {
private:
    mutex portMutex;
    int fileDescriptor;
    bool isConnect;
public:
    SerialPort();
    ~SerialPort();
    
    bool connect(const std::string& portName);
    void disconnect();
    bool sendData(const std::string& data);
    bool sendBytes(const std::vector<uint8_t>& data);
    int readBytes(std::vector<uint8_t>& buffer, int expectedLength, int timeout);
    // Очистка канала
    void flush();
};

#endif /* SerialPort_hpp */
