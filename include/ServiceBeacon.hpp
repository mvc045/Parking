//
//  ServiceBeacon.hpp
//  Parking
//
//  Created by Михаил Конюхов on 26.12.2025.
//

#ifndef ServiceBeacon_hpp
#define ServiceBeacon_hpp

#include <stdio.h>
#include <string>
#include <thread>
#include <atomic>
#include <netinet/in.h>

using namespace std;

class ServiceBeacon {
private:
    atomic<bool> running;
    thread worker;
    
    int sockFd;
    struct sockaddr_in broadcastAddr;
    
    int httpPort;
    int broadcastPort;
    
    string deviceId;
    
    void broadcastLoop();
public:
    ServiceBeacon(const string& id, int serverPort, int updPort = 30001);
    ~ServiceBeacon();
    
    bool start();
    void stop();
};

#endif /* ServiceBeacon_hpp */
