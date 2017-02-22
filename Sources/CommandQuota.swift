//
//  CommandQuota.swift
//  FireAlarm
//
//  Created by NobodyNada on 12/20/16.
//
//

import Foundation
import SwiftChatSE
import SwiftStack

class CommandQuota: Command {
	override class func usage() -> [String] {
		return ["quota", "api quota", "api-quota"]
	}
	
	override func run() throws {
		reply("API quota is \(apiClient.quota.map { String($0) } ?? "unknown").")
	}
}
