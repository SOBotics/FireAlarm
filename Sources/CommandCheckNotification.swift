//
//  CommandCheckNotification.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandCheckNotification: Command {
	override class func usage() -> [String] {
		return ["notified", "aminotified", "am i notified", "will i be notified", "opted-in", "opted in", "am i opted in", "have i opted in"]
	}
	
	override func run() throws {
		if !message.user.notified {
			reply("You will not be notified of any reports.")
		}
		else if message.user.notificationTags.isEmpty && message.user.notificationReasons.isEmpty {
			reply("You will be notified of all reports.")
		}
		else {
			var reasons: [String] = []
			if !message.user.notificationTags.isEmpty {
				let formattedTags = formatArray(message.user.notificationTags.map { "[tag:\($0)]" }, conjunction: "or")
				reasons.append("reports tagged "+formattedTags)
			}
			if message.user.notificationReasons.contains("username") {
				reasons.append("blacklisted usernames")
			}
			if message.user.notificationReasons.contains("misleadingLink") {
				reasons.append("misleading links")
			}
			let formatted = formatArray(reasons, conjunction: "or")
			reply("You will be notified of \(formatted).")
		}

	}
}
