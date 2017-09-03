//
//  CommandGetBlacklistedUsernames.swift
//  FireAlarm
//
//  Created by NobodyNada on 11/30/16.
//
//

import Foundation
import SwiftChatSE

class CommandGetBlacklistedUsernames: Command {
	override class func usage() -> [String] {
		return ["blacklisted usernames", "blacklisted users"]
	}
	
	override func run() throws {
		let filter = reporter.filter(ofType: FilterBlacklistedUsernames.self)
		
		if filter?.blacklistedUsernames.count == 0 {
			reply("No usernames are blacklisted.")
			return
		}
		reply("Blacklisted usernames:")
		post("    " + (filter?.blacklistedUsernames.joined(separator: "\n    ") ?? ""))
	}
}
