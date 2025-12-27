//
//  ServiceBeacon.cpp
//  Parking
//
//  Created by Михаил Конюхов on 26.12.2025.
//

#include "ServiceBeacon.hpp"
#include "json.hpp"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>

using namespace std;
using json = nlohmann::json;

ServiceBeacon::ServiceBeacon(const string& id, int http, int upd)
    : deviceId(id), httpPort(http), broadcastPort(upd), running(false), sockFd(-1) {}

ServiceBeacon::~ServiceBeacon() {
    stop();
}

bool ServiceBeacon::start() {
    if (running) return true;
    
    // Создаем UPD сокет
    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0) {
        cerr << "[ServiceBeacon] Ошбика upd сокета\n";
        return false;
    }
    
    // Включаем режим broadcast, что бы кричат ьна всю 255 сеть
    int broadcastEnable = 1;
    if (setsockopt(sockFd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        cerr << "[ServiceBeacon] Ошибка установки broadcast режима\n";
        close(sockFd);
        return false;
    }
    
    // Адрес получателя (255.255.255.255)
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(broadcastPort);
    broadcastAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    running = true;
    worker = thread(&ServiceBeacon::broadcastLoop, this);
    
    cout << "[ServiceBeacon] Работает...\n";
    return true;
}

void ServiceBeacon::stop() {
    running = false;
    
    if (sockFd != -1) {
        close(sockFd);
        sockFd = -1;
    }
    
    if (worker.joinable()) {
        worker.join();
    }
}

void ServiceBeacon::broadcastLoop() {
    while (running) {
        json j;
        j["device_id"] = deviceId;
        j["service_name"] = "Шлагбаум";
        j["ws_host"] = "ws://localhost:8081/ws";
        
        string msg = j.dump() + "\n";
        
        // Отправляем
        // {"device_id":"0","api_port":8081,"service_name":"Шлагбаум"}
        ssize_t sent = sendto(sockFd, msg.c_str(), msg.length(), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        if (sent < 0) {
            cerr << "[ServiceBeacon] Ошибка при рассылке пакетов..." << sent << "\n";
        }
        
        this_thread::sleep_for(chrono::seconds(2));
    }
}
