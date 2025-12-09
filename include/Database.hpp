//
//  Database.hpp
//  Parking
//
//  Created by Михаил Конюхов on 08.12.2025.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <sqlite3.h>
#include "json.hpp"

using namespace std;

class Database {
private:
    sqlite3* db;
    string path;
public:
    Database(const string& pathDb);
    ~Database();
    
    void logEvent(const string& type, const string& message, const int& deviceId);
    string getCurrentTime();
    nlohmann::json getHistory();
    
};

#endif /* Database_hpp */
