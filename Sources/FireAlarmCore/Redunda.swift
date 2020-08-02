//
//  Redunda.swift
//  FireAlarm
//
//  Created by NobodyNada on 3/23/17.
//
//

import Foundation
import SwiftChatSE
import Dispatch
import CryptoSwift

open class Redunda {
    public enum RedundaError: Error {
        case invalidJSON(json: Any)
        case downloadFailed(status: Int)
        case uploadFailed(status: Int)
    }
    
    
    public struct Event {
        public let name: String
        public let headers: [String:String]
        public let content: String
        
        public func contentAsJSON() throws -> Any {
            return try JSONSerialization.jsonObject(with: content.data(using: .utf8)!)
        }
        
        public init(json: [String:Any]) throws {
            guard let name = json["name"] as? String,
                let headers = json["headers"] as? [String:String],
                let content = json["content"] as? String
                else { throw RedundaError.invalidJSON(json: json) }
            
            self.name = name
            self.headers = headers
            self.content = content
        }
    }
	
	public let key: String
	public let client: Client
	public let filesToSync: [String]	//An array of regexes.
	
	
	open func downloadFile(named name: String) throws {
		print("Downloading \(name).")
		
		let (data, response) = try client.get("https://redunda.sobotics.org/bots/data/\(name)?key=\(key)")
		
		guard response.statusCode == 200 else {
			throw RedundaError.downloadFailed(status: response.statusCode)
		}
		
		try data.write(to:
			URL(fileURLWithPath: FileManager.default.currentDirectoryPath).appendingPathComponent(name)
		)
	}
	
	open func uploadFile(named name: String) throws {
		print("Uploading \(name).")
		
		let data = try Data(contentsOf:
			URL(fileURLWithPath: FileManager.default.currentDirectoryPath).appendingPathComponent(name)
		)
		
		let (_, response) = try client.post(
			"https://redunda.sobotics.org/bots/data/\(name)?key=\(key)",
			data: data, contentType: "application/octet-stream"
		)
		
		guard response.statusCode < 400 else {
			throw RedundaError.uploadFailed(status: response.statusCode)
		}
	}
	
	
	open func hash(of file: String) throws -> String {
        return try Data(contentsOf: URL(fileURLWithPath: file)).sha256().toHexString()
	}
	
	///Downloads modified files from Redunda.
	///- Warning:
	///Do not post non-`RedundaError`s to chat; they may contain the instance key!
	open func downloadFiles() throws {
		let response = try client.parseJSON(client.get("https://redunda.sobotics.org/bots/data.json?key=\(key)"))
		
		guard let json = response as? [[String:Any]] else {
			throw RedundaError.invalidJSON(json: response)
		}
		
		let manager = FileManager.default
		
		for item in json {
			guard let filename = item["key"] as? String else {
				throw RedundaError.invalidJSON(json: response)
			}
			if manager.fileExists(atPath: filename) {
				if try hash(of: filename) != item["sha256"] as? String {
					try downloadFile(named: filename)
				}
			} else {
				try downloadFile(named: filename)
			}
		}
	}
	
	///Downloads modified files from Redunda.
	///- Warning:
	///Do not post non-`RedundaError`s to chat; they may contain the instance key!
	open func uploadFiles() throws {
		let response = try client.parseJSON(client.get("https://redunda.sobotics.org/bots/data.json?key=\(key)"))
		
		guard let json = response as? [[String:Any]] else {
			throw RedundaError.invalidJSON(json: response)
		}
		
		let manager = FileManager.default
		
		for filename in try manager.contentsOfDirectory(atPath: ".") {
			for regex in filesToSync {
				guard filename.range(of: regex, options: [.regularExpression]) != nil else {
					continue
				}
				
                if let index = json.firstIndex(where: { filename == $0["key"] as? String }) {
					if try hash(of: filename) != json[index]["sha256"] as? String {
						try uploadFile(named: filename)
					}
				} else {
					try uploadFile(named: filename)
				}
			}
		}
	}
	
	public init(key: String, client: Client, filesToSync: [String] = []) {
		self.key = key
		self.client = client
		self.filesToSync = filesToSync
	}
	
	open var shouldStandby: Bool = false
	open var locationName: String?
    
    //The number of unread events.
    open var eventCount: Int = 0
    
    open func fetchEvents() throws -> [Event] {
        let json = try client.parseJSON(
            try client.post(
                "https://redunda.sobotics.org/events.json",
                ["key":key]
            )
        )
        
        guard let events = try (json as? [[String:Any]])?.map(Event.init) else {
            throw RedundaError.invalidJSON(json: json)
        }
        
        eventCount = 0
        return events
    }
	
	open func sendStatusPing(version: String? = nil) throws {
		let data = version == nil ? ["key":key] : ["key":key, "version":version!]
		let response = try client.parseJSON(try client.post("https://redunda.sobotics.org/status.json", data))
		guard let json = response as? [String:Any] else {
			throw RedundaError.invalidJSON(json: response)
		}
		
		guard let standby = json["should_standby"] as? Bool else {
			throw RedundaError.invalidJSON(json: response)
		}
        
        guard let eventCount = json["event_count"] as? Int else {
            throw RedundaError.invalidJSON(json: response)
        }
		
		shouldStandby = standby
		locationName = json["location"] as? String
        self.eventCount = eventCount
	}
}
