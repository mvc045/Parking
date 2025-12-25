//
//  NetworkServer.cpp
//  Parking
//
//  Created by Михаил Конюхов on 24.12.2025.
//

#include "NetworkServer.hpp"
#include <iostream>
#include <future>
#include <memory>

using namespace std;
using json = nlohmann::json;

NetworkServer::NetworkServer(GateController& gc, Database& db, const string& key): controller(gc), db(db), apiKey(key) {
}

void NetworkServer::start(int port) {
    // uWebSocket захватыает поток, поэтому запустим его в отдельнм thread
    serverThread = thread([this, port]() {
        
        this->loop = uWS::Loop::get();
        
        uWS::App app;
        this->globalApp = &app;
        
        // MARK: REST API
        app.get("/status", [this](auto* res, auto* req) {
            bool isOpen = controller.isGateOpen();
            
            json response;
            response["device_id"] = 0;
            response["status"] = isOpen ? "open" : "closed";
            response["timestamp"] = time(nullptr);
            
            int percent = this->controller.getGatePosition();
            
            if (percent >= 0) {
                response["position"] = percent;
            } else {
                response["position"] = nullptr;
            }
                        
            res->writeHeader("Content-Type", "application/json");
            res->end(response.dump());
        });
        
        app.get("/history", [this](auto* res, auto* req) {
            json response = db.getHistory();
            
            res->writeHeader("Content-Type", "application/json");
            res->end(response.dump());
        });
        
        app.post("/open", [this](auto* res, auto* req) {
            string authToken = string(req->getHeader("authorization"));
            
            if (authToken.find(this->apiKey) == string::npos) {
                json response;
                response["ok"] = false;
                res->writeStatus("401 Unauthorized")->writeHeader("Content-Type", "application/json")->end(response.dump());
                return;
            }
            
            thread([this]() {
                try {
                    this->controller.openGate();
                } catch (const exception& e) {
                    // Отлавливаем ошибки
                    cerr << "Ошибка: " << e.what();
                }
            }).detach();
            
            json response;
            response["ok"] = true;
            response["status"] = "accepted";
            
            res->writeHeader("Content-Type", "application/json")->end(response.dump());
        });
        
        app.post("/close", [this](auto* res, auto* req) {
            string authToken = string(req->getHeader("authorization"));
            
            if (authToken.find(this->apiKey) == string::npos) {
                json response;
                response["ok"] = false;
                res->writeStatus("401 Unauthorized")->writeHeader("Content-Type", "application/json")->end(response.dump());
                return;
            }
            
            thread([this]() {
                try {
                    this->controller.closeGate();
                } catch (const exception& e) {
                    // Отлавливаем ошибки
                    cerr << "Ошибка: " << e.what();
                }
            }).detach();
            
            json response;
            response["ok"] = true;
            response["status"] = "accepted";
            
            res->writeHeader("Content-Type", "application/json")->end(response.dump());
        });
        
        app.post("/rfid/user", [this](auto* res, auto* req) {
            string authToken = string(req->getHeader("authorization"));
            
            if (authToken.find(this->apiKey) == string::npos) {
                json response;
                response["ok"] = false;
                res->writeStatus("401 Unauthorized")->writeHeader("Content-Type", "application/json")->end(response.dump());
                return;
            }
            
            postJSON(res, [this](json body) -> json {
                string user = body["username"];
                string cardCode = body["card_code"];
                
                auto result = this->db.createRFIDCard(user, cardCode);
                
                json localResponse;
                                
                switch (result) {
                    case RFIDCardCreationResult::Success:
                        localResponse["ok"] = true;
                        localResponse["cardCode"] = cardCode;
                        break;
                    case RFIDCardCreationResult::ErrorNameExists:
                        localResponse["ok"] = false;
                        localResponse["message"] = "Такой username уже существует";
                        break;
                    case RFIDCardCreationResult::ErrorCodeExists:
                        localResponse["ok"] = false;
                        localResponse["message"] = "Такой RFID ключ уже сущетсвует";
                        break;
                    case RFIDCardCreationResult::Error:
                        localResponse["ok"] = false;
                        localResponse["message"] = "Неизвестная ошибка";
                        break;
                    default:
                        break;
                }
                
                return localResponse;
            });
            
        });
        
        // MARK: WebSocket
        app.ws<PerSocketData>("/ws", {
            .compression = uWS::SHARED_COMPRESSOR,
            .maxPayloadLength = 16 * 1024,
            .idleTimeout = 16,
            .open = [](auto* ws) {
                ws->subscribe("broadcast");
                cout << "[WS] Клиент подключился\n";
            },
            .message = [](auto* ws, string_view message, uWS::OpCode opCode) {
                // Обрабатываем входящие сообщения
                cout << message << "\n";
            }
        });
        
        app.listen(port, [port](auto* token){
            if (token) {
                cout << "[uWS] Сервер запущен на " << port << " порту.\n";
            } else {
                cout << "[uWS] Порт: " << port << " занят.\n";
            }
        });
        
        app.run();
        
    });
    
    serverThread.detach();
}

void NetworkServer::broadcastEvent(const string& eventType, const json& data) {
    if (!loop || !globalApp) return;
    
    json payload;
    payload["event"] = eventType;
    payload["data"] = data;
    payload["timestamp"] = time(nullptr);
    
    string message = payload.dump();
    
    loop->defer([this, message]() {
        this->globalApp->publish("broadcast", message, uWS::OpCode::TEXT, false);
    });
}

void NetworkServer::postJSON(uWS::HttpResponse<false>* res, JSONHandler handler) {
    res->onAborted([]() {
        cout << "[uWS] Обрыв соединения\n";
    });
    
    res->onData([res, handler, buffer = string()](string_view chunk, bool isLast) mutable {
        buffer.append(chunk);
        
        if (isLast) {
            try {
                if (buffer.empty()) throw runtime_error("Empty Body");
                json reqJson = json::parse(buffer);
                json respJson = handler(reqJson);
                
                res->writeHeader("Content-Type", "application/json")->end(respJson.dump());
            } catch (const exception& e) {
                json err;
                err["ok"] = false;
                err["message"] = e.what();
                res->writeHeader("Content-Type", "application/json")->end(err.dump());
            }
        }
    });
}
