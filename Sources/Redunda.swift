//
//  Redunda.swift
//  FireAlarm
//
//  Created by NobodyNada on 3/23/17.
//
//

import Foundation
import SwiftChatSE

class Redunda {
	enum RedundaError: Error {
		case invalidJSON(json: Any)
		case downloadFailed(status: Int)
	}
	
	let key: String
	let client: Client
	let filesToSync: [String]	//An array of regexes.
	
	
	func downloadFile(named name: String) throws {
		print("Downloading \(name).")
		
		let (data, response) = try client.get("https://redunda.sobotics.org/bots/data/\(name)?key=\(key)")
		
		guard response.statusCode == 200 else {
			throw RedundaError.downloadFailed(status: response.statusCode)
		}
		
		try data.write(to:
			URL(fileURLWithPath: FileManager.default.currentDirectoryPath).appendingPathComponent(name)
		)
	}
	
	func uploadFile(named name: String) throws {
		print("Uploading \(name).")
		
		let data = try Data(contentsOf: URL(fileURLWithPath: path))
		
		let (_, response) = try client.post("https://redunda.sobotics.org/bots/data/\(name)?key=\(key)", data)
	}
	
	
	func hash(of file: String) -> String {
		let pipe = Pipe()
		let process = Process()
		process.launchPath = "/usr/bin/env"
		process.arguments = ["shasum", "-a", "256", file]
		process.standardOutput = pipe
		process.launch()
		
		var hash: String?
		
		let sema = DispatchSemaphore(value: 0)
		
		process.terminationHandler = {process in
			if let info = String(data: pipe.fileHandleForReading.readDataToEndOfFile(), encoding: .utf8) {
				hash = info.trimmingCharacters(in: .whitespacesAndNewlines).components(separatedBy: .whitespaces).first
			}
			sema.signal()
		}
		
		let _ = sema.wait(timeout: DispatchTime.now() + DispatchTimeInterval.seconds(5))
		
		if hash != nil {
			return hash!
		} else {
			return "<failed to get hash>"
		}
	}
	
	///Downloads modified files from Redunda.
	///- Warning:
	///Do not post errors to chat; they contain the instance key!
	func downloadFiles() throws {
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
				if hash(of: filename) != item["sha256"] as? String {
					try downloadFile(named: filename)
				}
			} else {
				try downloadFile(named: filename)
			}
		}
	}
	
	///Downloads modified files from Redunda.
	///- Warning:
	///Do not post errors to chat; they contain the instance key!
	func uploadFiles() throws {
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
				
				if let index = json.index(where: { filename == $0["key"] as? String }) {
					if hash(of: filename) != json[index]["sha256"] as? String {
						try uploadFile(named: filename)
					}
				} else {
					try uploadFile(named: filename)
				}
			}
		}
	}
	
	init(key: String, client: Client, filesToSync: [String] = []) {
		self.key = key
		self.client = client
		self.filesToSync = filesToSync
	}
	
	var shouldStandby: Bool = false
	
	func sendStatusPing(version: String? = nil) throws {
		let data = version == nil ? ["key":key] : ["key":key, "version":version!]
		let response = try client.parseJSON(try client.post("https://redunda.sobotics.org/status.json", data))
		guard let json = response as? [String:Any] else {
			throw RedundaError.invalidJSON(json: response)
		}
		
		guard let standby = json["should_standby"] as? Bool else {
			throw RedundaError.invalidJSON(json: response)
		}
		
		shouldStandby = standby
	}
}
