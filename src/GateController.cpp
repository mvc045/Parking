//
//  GateController.cpp
//  Parking
//
//  Created by Михаил Конюхов on 03.12.2025.
//

#include "GateController.hpp"
#include "ModbusUtils.hpp"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <stdexcept>

using namespace std;

void GateController::openGate() {
    cout << "[Controller] Отправили команду на открытие\n";
    
    ModbusFrame frame = { deviceId, Command::WRITE_SINGL_COIL, 0x0000, Action::OPEN };
    auto rawData = frame.serialize();

    port.sendBytes(rawData);
    cout << "Отправили пакет Modbus размером " << rawData.size() << " байт\n";

    // по стандарту modbus, устройство должно прислать ответ (ACK)
    vector<uint8_t> response;
    int bytesRead = port.readBytes(response, 8, 2); // Ждем 8 байт, 2 секунды

    // Проверяем что вернулось 8 байт
    if (bytesRead != 8) {
        cerr << "[Controller] Ошибка, с ответом от шлагбаума что то не так\n";
        cerr << "[Controller] " << bytesRead << "\n";
        
        throw runtime_error("Ошибка, с ответом от шлагбаума что то не так");
        return;
    }

    // Проверяем СRС ответа, нужно что бы они совпадали
    uint16_t receivedCRC = response[6] | (response[7] << 8);
    vector<uint8_t> dataOnly(response.begin(), response.end() - 2);
    uint16_t calcCRC = ModbusUtils::calculateCRC(dataOnly);

    if (receivedCRC == calcCRC) {
        cout << "[Controller] CRC совпадают\n";
        cout << "[Controller] Шлагбаум начал открываться\n";
        waitForOpen();
    } else {
        cerr << "[Controller] CRC не совпали\n";
        throw runtime_error("CRC не совпали");
    }
}

bool GateController::isGateOpen() {
    port.flush();
    // Адрес DI2 0x0002 (на открытие)
    ModbusFrame frame = { deviceId, Command::READ_DSSCRETE_INPUTS, 0x0002, Action::SINGLE };
    auto rawData = frame.serialize();
    
    if (!port.sendBytes(rawData)) return false;
    
    vector<uint8_t> response;
    int bytesRead = port.readBytes(response, 6, 2); // Ждем 6 байт, 2 секунды
    
    if (bytesRead != 6) {
        return  false;
    }
    
    // Сверяем что ответ именно на нашу команду
    if (response[1] != 0x02) {
        cerr << "[Polling] Пришел неожиданный пакет. FC=" << static_cast<int>(response[1]) << "\n";
        return false;
    }
    
    // Парсим биты
    // -------------
    // response[0] = ID
    // response[1] = 0x02
    // response[2] = Кол-во байт данных (должно быть 1)
    
    // response[3] = Данные (Битовая маска)
    
    uint8_t statusByte = response[3];
    bool isOpen = (statusByte & 0x01) != 0;
    // cout << "[Polling] statusByte - " << (int)response[3] << "\n";
    return isOpen;
}

void GateController::waitForOpen() {
    int timeoutSeconds = 10;
    
    auto startTime = chrono::steady_clock::now();
    
    while (true) {
        
        if (GateController::isGateOpen()) {
            cout << "[Polling] Шлагбаум открыт \n";
            port.flush();
            break;
        }
        
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - startTime).count() > timeoutSeconds) {
            cout << "[Polling] Отвалились по таймауту \n";
            break;
        }
        
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void GateController::closeGate() {
    cout << "[Controller] Отправили команду на закрытие\n";

    ModbusFrame frame = { deviceId, Command::WRITE_SINGL_COIL, 0x0000, Action::CLOSE };
    auto rawData = frame.serialize();
    port.sendBytes(rawData);
    
    waitForClose();
}

void GateController::waitForClose() {
    int timeoutSeconds = 10;
    
    auto startTime = chrono::steady_clock::now();
    
    while (true) {
        
        if (GateController::isGateClose()) {
            cout << "[Polling] Шлагбаум закрыт \n";
            break;
        }
        
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - startTime).count() > timeoutSeconds) {
            cout << "[Polling] Отвалились по таймауту \n";
            break;
        }
        
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
}

bool GateController::isGateClose() {
    port.flush();
    // Адрес DI1 0x0001 (на закрытие)
    ModbusFrame frame = { deviceId, Command::READ_DSSCRETE_INPUTS, 0x0001, Action::SINGLE };
    auto rawData = frame.serialize();
    
    if (!port.sendBytes(rawData)) return false;
    
    vector<uint8_t> response;
    int bytesRead = port.readBytes(response, 6, 2); // Ждем 6 байт, 2 секунды
    
    if (bytesRead != 6) {
        return  false;
    }
    
    // Сверяем что ответ именно на нашу команду
    if (response[1] != 0x02) {
        cerr << "[Polling] Пришел неожиданный пакет. FC=" << static_cast<int>(response[1]) << "\n";
        return false;
    }
    
    uint8_t statusByte = response[3];
    bool isClose = (statusByte & 0x01) != 0;
    // cout << "[Polling] statusByte - " << (int)response[3] << "\n";
    return isClose;
}
