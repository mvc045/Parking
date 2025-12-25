//
//  SerialPort.cpp
//  Parking
//
//  Created by Михаил Конюхов on 01.12.2025.
//

#include "SerialPort.hpp"
#include "ICommunication.h"
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <errno.h>
#include <termios.h> // POSIX terminal control
#include <unistd.h>
#include <thread>

using namespace std;

SerialPort::SerialPort() : fileDescriptor(-1), isConnect(false) {}

SerialPort::~SerialPort() {}

bool SerialPort::connect(const string& portName) {
    
    // Открываем порт как обычный файл
    // O_RDWR - чтение и запись
    // O_NOCTTY - порт не станет управляющим терминалом процесса
    // O_NDELAY - не ждать DCD линии (важно для эмуляторов)
    fileDescriptor = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (fileDescriptor == -1) {
        // Не смогли подключиться
        return false;
    }
    
    struct termios tty;
    
    if (tcgetattr(fileDescriptor, &tty) != 0) {
        // Тож ошибка
        return false;
    }
    
    // Скорость input и output
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);
    
    // 8 бит данных
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    
    // Отключаем программное управление потоком
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    // Отключаем обработку спецсимволов (наример Ctrl+C)
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    if (tcsetattr(fileDescriptor, TCSANOW, &tty) != 0) {
        // Ошибка настройки почему то не принялись
        return false;
    }
    
    isConnect = true;
    // Снимаем блокировку
    fcntl(fileDescriptor, F_SETFL, 0);
    
    std::cout << "Успешно подключились к: " << portName << std::endl;
    return true;
}

void SerialPort::disconnect() {
    if (isConnect && fileDescriptor != -1) {
        close(fileDescriptor);
        isConnect = false;
        std::cout << "Отключились." << std::endl;
    }
}

bool SerialPort::sendBytes(const vector<uint8_t>& data) {
    if (!isConnect) {
        return false;
    }
    
    // блокируем поток
    lock_guard<mutex> lock(portMutex);
    
    // кол-во записаных байт
    ssize_t bytesWritte = write(fileDescriptor, data.data(), data.size());
    
    // если записали не столько сколько хотели, ошибка
    if (bytesWritte != (ssize_t)data.size()) {
        cout << "Ошибка, записалось не столько байт сколько ожидалось";
        return false;
    }
    
    return true;
}

int SerialPort::readBytes(vector<uint8_t>& buffer, int expectedLength, int timeoutSec) {
    if (!isConnect) {
        return -1;
    }
    // блокируем поток
    lock_guard<mutex> lock(portMutex);
    
    buffer.clear();
    buffer.reserve(expectedLength);
    
    int totalBytesRead = 0;
    
    uint8_t tempBuffer[256];
    
    while (totalBytesRead < expectedLength) {
        fd_set readfts;
        FD_ZERO(&readfts);
        FD_SET(fileDescriptor, &readfts);
        
        struct timeval timeout;
        timeout.tv_sec = timeoutSec;
        timeout.tv_usec = 0;
        
        // ждем пока придет ответ, или отваливаем по таймауту
        int result = select(fileDescriptor + 1, &readfts, NULL, NULL, &timeout);
        
        if (result < 0) {
            cout << "Ошибка в ACK\n";
            return -1;
        } 
        
        if (FD_ISSET(fileDescriptor, &readfts)) {
            int bytesToRead = expectedLength - totalBytesRead;
            if (bytesToRead > sizeof(tempBuffer)) bytesToRead = sizeof(tempBuffer);
            
            ssize_t n = read(fileDescriptor, tempBuffer, bytesToRead);
            if (n > 0) {
                buffer.insert(buffer.end(), tempBuffer, tempBuffer + n);
                totalBytesRead += n;
            } else {
                break;
            }
        }
        
    }
    
    return totalBytesRead;
    
}

void SerialPort::flush() {
    lock_guard<mutex> lock(portMutex);
    if (fileDescriptor == -1) return;
    
    int flags = fcntl(fileDescriptor, F_GETFL, 0);
    fcntl(fileDescriptor, F_SETFL, flags | O_NONBLOCK);
    
    uint8_t buffer[1024];
    int totalBytes = 0;
    int bytesRead = 0;
    
    do {
        bytesRead = read(fileDescriptor, buffer, sizeof(buffer));
        if (bytesRead > 0) totalBytes += bytesRead;
        if (bytesRead > 0) this_thread::sleep_for(chrono::milliseconds(10));
    } while (bytesRead > 0);
    
    fcntl(fileDescriptor, F_SETFL, flags);
    
    if (totalBytes > 0) {
        cout << "[SerialPort] Очистка мусора " << totalBytes << " байт мусора.\n";
    }
}
