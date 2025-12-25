//
//  BarrierViewModel.swift
//  ParkingClient-iOS
//
//  Created by Михаил Конюхов on 25.12.2025.
//

import Foundation

class BarrierViewModel {
    
    private let socketService: SocketService
    private(set) var messages: [String] = []
    
    var onMessagesUpdated: (() -> Void)?
    var onStatusChanged: ((Bool) -> Void)?
    
    private(set) var isOpen: Bool = false
    private(set) var position: Int = 0
    
    init() {
        socketService = SocketService()
        socketService.delegate = self
    }
    
    func connect() {
        socketService.connect()
    }
    
    func sendMessage(text: String) {
        socketService.sendMessage(text)
    }
    
}

extension BarrierViewModel: SocketServiceDelegate {
    
    func didReceive(message: String) {
        messages.append(message)
        
        if let data = message.data(using: .utf8), let messageModel = try? JSONDecoder().decode(Message.self, from: data) {
            switch messageModel.event {
            case .status:
                isOpen = messageModel.data?.state == "Open" ? true : false
            case .update:
                if let position = messageModel.data?.position {
                    self.position = position
                }
            default:
                break
            }
        }

        onMessagesUpdated?()
    }
    
    func didChangeStatus(isConnected: Bool) {
        onStatusChanged?(isConnected)
    }
    
}
