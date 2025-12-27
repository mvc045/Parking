//
//  UDPListener.swift
//  ParkingClient-iOS
//
//  Created by Михаил Конюхов on 27.12.2025.
//

import Foundation
import Network

protocol UDPListenerDelegate: AnyObject {
    func didReceive(message: MessageUPDBroadcast)
}

public final class UDPListener {
    
    private var listener: NWListener?
    private let queue = DispatchQueue(label: "udp.listener.queue")
    
    weak var delegate: UDPListenerDelegate?
        
    func startListenr(port: UInt16) {
        do {
            let params = NWParameters.udp
            params.allowLocalEndpointReuse = true
            
            
            listener = try NWListener(using: params, on: .init(rawValue: port)!)
            listener?.newConnectionHandler = { [weak self] connection in
                self?.handleConnection(connection)
            }
            listener?.stateUpdateHandler = { state in
                switch state {
                case .ready:
                    print("Слушатель готов на порту \(port)")
                case .failed(let error):
                    print("Ошибка: \(error.localizedDescription)")
                default:
                    break
                }
            }
            listener?.start(queue: queue)
        } catch {
            print("Ошибка")
        }
    }
    
    func handleConnection(_ connection: NWConnection) {
        connection.start(queue: queue)
        
        connection.receiveMessage { [weak self] (data, context, isComplete, error) in
            if let data, !data.isEmpty, let message = try? JSONDecoder().decode(MessageUPDBroadcast.self, from: data) {
                self?.delegate?.didReceive(message: message)
            }
            
            if let error {
                print("Ошибка при получении данных: \(error.localizedDescription)")
            }
        }
    }
    
    func stop() {
        listener?.cancel()
        listener = nil
    }
    
}
