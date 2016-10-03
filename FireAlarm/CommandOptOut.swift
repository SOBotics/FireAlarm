//
//  CommandOptOut.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandOptOut: Command {
	override class func usage() -> [String] {
		return ["opt out ...", "opt-out ..."]
	}
	
	override func run() throws {
		if arguments.isEmpty || message.user.notificationTags.isEmpty {
			message.user.notified = false
			message.user.notificationTags = []
			bot.room.postReply("You will not be notified of any reports.", to: message)
		}
		else {
			message.user.notificationTags = message.user.notificationTags.filter { !arguments.contains($0) }
			
			let formatted = formatArray(arguments.map { "[tag:\($0)]" }, conjunction: "or")
			bot.room.postReply("You will not be notified of posts tagged \(formatted).", to: message)
		}
	}
}
