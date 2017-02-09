//
//  CommandBlacklistUsername.swift
//  FireAlarm
//
//  Created by NobodyNada on 11/30/16.
//
//

import Foundation
import SwiftChatSE

class CommandBlacklistUsername: Command {
	override class func usage() -> [String] {
		return ["blu ...", "blacklist user ...", "blacklist username ..."]
	}
	
	override class func privileges() -> ChatUser.Privileges {
		return .filter
	}
	
	
	override func run() throws {
		if arguments.count == 0 {
			reply("Please sepcify the username to blacklist.")
			return
		}
		let regex = arguments.joined(separator: " ")
		filter.blacklistedUsernames.append(regex)
		reply("Blacklisted regular expression `\(regex)`.")
	}
}
