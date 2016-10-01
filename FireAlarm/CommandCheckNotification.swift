//
//  CommandCheckNotification.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Cocoa

class CommandCheckNotification: Command {
	override class func usage() -> [String] {
		return ["notified", "aminotified", "am i notified", "will i be notified", "opted-in", "opted in", "am i opted in", "have i opted in"]
	}
	
	override func run() throws {
		if !message.user.notified {
			bot.room.postReply("You will not notified of any reports.", to: message)
		}
		else if message.user.notificationTags.isEmpty {
			bot.room.postReply("You will be notified of all reports.", to: message)
		}
		else {
			var string = ""
			if message.user.notificationTags.count == 1 {
				string = "[tag:\(arguments.first!)]"
			}
			else {
				for tag in message.user.notificationTags {
					if tag == message.user.notificationTags.last {
						string.append("or [tag:\(tag)]")
					}
					else {
						string.append("[tag:\(tag), ")
					}
				}
			}
			bot.room.postReply("You will be notified of reports tagged \(string).", to: message)
		}

	}
}
