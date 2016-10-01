//
//  CommandOptOut.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Cocoa

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
			
			var string = ""
			if arguments.count == 1 {
				string = "[tag:\(arguments.first!)]"
			}
			else {
				for tag in arguments {
					if tag == message.user.notificationTags.last {
						string.append("or [tag:\(tag)]")
					}
					else {
						string.append("[tag:\(tag), ")
					}
				}
			}
			
			bot.room.postReply("You will not be notified of posts tagged \(string).", to: message)
		}
	}
}
