//
//  GateController.hpp
//  Parking
//
//  Created by Михаил Конюхов on 03.12.2025.
//

#ifndef GateController_hpp
#define GateController_hpp

#include <stdio.h>
#include "SerialPort.hpp"
#include "ICommunication.h"
#include <unistd.h>
#include <functional>

using namespace std;

class GateController {
    using LogCallback = function<void(string type, string message)>;
private:
    ICommunication& port;
    LogCallback logger;
    uint8_t deviceId;
public:
    GateController(ICommunication& channel, uint8_t id) : port(channel), deviceId(id) {}
    
    void setLogger(LogCallback cb) {
        logger = cb;
    }
    
    void log(const string& type, const string& msg) {
        if (logger) {
            logger(type, msg);
        }
    }
    
    void openGate(bool autoClose = false);
    void waitForOpen();
    bool isGateOpen();
    void closeGate();
    void waitForClose();
    bool isGateClose();
    int  getGatePosition();
};
#endif /* GateController_hpp */
