//
//  Message.swift
//  ParkingClient-iOS
//
//  Created by Михаил Конюхов on 25.12.2025.
//


struct Message: Codable {
    
	let data : Data?
	let event : Event?
	let timestamp : Int?

	enum CodingKeys: String, CodingKey {
		case data = "data"
		case event = "event"
		case timestamp = "timestamp"
	}

	init(from decoder: Decoder) throws {
		let values = try decoder.container(keyedBy: CodingKeys.self)
		data = try values.decodeIfPresent(Data.self, forKey: .data)
        event = try values.decodeIfPresent(Event.self, forKey: .event)
		timestamp = try values.decodeIfPresent(Int.self, forKey: .timestamp)
	}
    
    enum Event: String, Codable {
        case status = "GATE_STATUS"
        case update = "GATE_UPDATE"
    }

}

struct Data: Codable {
    
    let state : String?
    let position: Int?
    
    enum CodingKeys: String, CodingKey {
        case state = "state"
        case position = "position"
    }
    
    init(from decoder: Decoder) throws {
        let values = try decoder.container(keyedBy: CodingKeys.self)
        state = try values.decodeIfPresent(String.self, forKey: .state)
        position = try values.decodeIfPresent(Int.self, forKey: .position)
    }
    
}

struct MessageUPDBroadcast: Codable {
    
    let deviceId : String?
    let serviceName : String?
    let wsHost : String?

    enum CodingKeys: String, CodingKey {
        case deviceId = "device_id"
        case serviceName = "service_name"
        case wsHost = "ws_host"
    }

    init(from decoder: Decoder) throws {
        let values = try decoder.container(keyedBy: CodingKeys.self)
        deviceId = try values.decodeIfPresent(String.self, forKey: .deviceId)
        serviceName = try values.decodeIfPresent(String.self, forKey: .serviceName)
        wsHost = try values.decodeIfPresent(String.self, forKey: .wsHost)
    }
    
}
