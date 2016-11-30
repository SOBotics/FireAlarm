//
//  ComandUnblacklistUsername.swift
//  FireAlarm
//
//  Created by NobodyNada on 11/30/16.
//
//

import Foundation
import SwiftChatSE

class CommandUnblacklistUsername: Command {
	override class func usage() -> [String] {
		return ["unblacklist username ...", "unblacklist user ...", "unblu ...", "rmblu ..."]
	}
	
	override class func privileges() -> ChatUser.Privileges {
		return [.blacklist]
	}
	
	override func run() throws {
		if arguments.count == 0 {
			reply("Please specify the username to remove from the blacklist.")
			return
		}
		
		let username = arguments.joined(separator: " ")
		if let index = filter.blacklistedUsernames.index(of: username) {
			filter.blacklistedUsernames.remove(at: index)
			reply("\(username) was removed from the username blacklist.")
		} else {
			reply("\(username) was not blacklisted.")
		}
	}
}
