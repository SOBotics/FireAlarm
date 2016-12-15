//
//  CommandOptIn.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandOptIn: Command {
	override class func usage() -> [String] {
		return ["notify ...", "opt in ...", "opt-in ..."]
	}
	
	func addNotificationTags() {
		if arguments.count < 2 {
			reply("Please specify which tags.")
			return
		}
		message.user.notified = true
		for tag in arguments.dropFirst() {
			if !message.user.notificationTags.contains(tag) {
				message.user.notificationTags.append(tag)
			}
		}
		let formatted = formatArray(message.user.notificationTags.map { "[tag:\($0)]" }, conjunction: "or")
		reply("You will now be notified of reports tagged \(formatted).")
	}
	
	func addNotificationUsernames() {
		if arguments.count > 1 {
			reply("Too many arguments.")
			return
		}
		message.user.notified = true
		if message.user.notificationReasons.contains("username") {
			reply("You're already notified for blacklisted username reports.")
		} else {
			message.user.notificationReasons.append("username")
			reply("You will now be notified for blacklisted username reports.")
		}
	}
	
	func addNotificationLinks() {
		message.user.notified = true
		if message.user.notificationReasons.contains("misleadingLink") {
			reply("You're already notified for misleading links.")
		} else {
			message.user.notificationReasons.append("misleadingLink")
			reply("You will now be notified for misleading links.")
		}
	}
	
	override func run() throws {
		
		if arguments.count == 0 {
			message.user.notified = true
			message.user.notificationTags = []
			message.user.notificationReasons = []
			reply("You will now be notified of all reports.")
		}
		else {
			switch arguments.first! {
			case "tag", "tags":
				addNotificationTags()
			case "username", "usernames":
				addNotificationUsernames()
			case "misleading", "misleadinglink", "misleadinglinks":
				addNotificationLinks()
			default:
				reply("Unrecognized reason \"\(arguments.first!)\"; allowed reasons are `tags`, `usernames`, and `misleading links`.")
			}
		}
		
	}
}
