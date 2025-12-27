//
//  SocketService.swift
//  ParkingClient-iOS
//
//  Created by Михаил Конюхов on 25.12.2025.
//

import Foundation
import Starscream

protocol SocketServiceDelegate: AnyObject {
    func didReceive(message: String)
    func didChangeStatus(isConnected: Bool)
}

class SocketService {
    
    private var socket: WebSocket?
    weak var delegate: SocketServiceDelegate?
        
    func connect(host: String) {
        var request = URLRequest(url: URL(string: host)!)
        request.timeoutInterval = 5
        socket = WebSocket(request: request)
        socket?.delegate = self
        socket?.connect()
    }
    
    func disconnect() {
        socket?.disconnect()
    }
    
    func sendMessage(_ text: String) {
        socket?.write(string: text)
    }
    
}

extension SocketService: WebSocketDelegate {
    
    func didReceive(event: WebSocketEvent, client: WebSocketClient) {
        switch event {
        case .connected:
            delegate?.didChangeStatus(isConnected: true)
        case .disconnected:
            delegate?.didChangeStatus(isConnected: false)
        case .text(let message):
            delegate?.didReceive(message: message)
        case .error(let error):
            delegate?.didReceive(message: "Ошибка: \(String(describing: error))")
        default:
            break
        }
    }
    
}
