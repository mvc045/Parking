//
//  main.cpp
//  Parking
//
//  Created by Михаил Конюхов on 01.12.2025.
//

#include "ConfigLoader.hpp"
#include <iostream>
#include <unistd.h>
#include "ParkingSystem.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    string configPath = "/Users/mvc/Documents/C++/SysCalls/Parking/config.txt";
    
    ParkingSystem app;
    
    if (!app.init(configPath)) {
        return 1;
    }
    
    app.run();
    
    return 0;
}

// cmake -G Xcode -B build_xcode
