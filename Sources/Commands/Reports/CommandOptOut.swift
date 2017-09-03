//
//  CommandOptOut.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandOptOut: Command {
	override class func usage() -> [String] {
		return ["opt out ...", "opt-out ..."]
	}
	
	func removeNotificationTags() {
		message.user.notificationTags = message.user.notificationTags.filter { !arguments.contains($0) }
		
		let formatted = formatArray(arguments.map { "[tag:\($0)]" }, conjunction: "or")
		reply("You will not be notified of posts tagged \(formatted).")
	}
	
	func removeNotificationUsernames() {
		guard let index = message.user.notificationReasons.index(of: "username") else {
			reply("You are not currently notified of blacklisted username reports.")
			return
		}
		message.user.notificationReasons.remove(at: index)
		reply("You will not be notified of blacklisted username reports.")
	}
	
	func removeNotificationLinks() {
		guard let index = message.user.notificationReasons.index(of: "misleadingLink") else {
			reply("You are not currently notified of misleading link reports.")
			return
		}
		message.user.notificationReasons.remove(at: index)
		reply("You will not be notified of misleading link reports.")
	}
	
	override func run() throws {
		if arguments.isEmpty || message.user.notificationTags.isEmpty {
			message.user.notified = false
			message.user.notificationTags = []
			message.user.notificationReasons = []
			reply("You will not be notified of any reports.")
		}
		else {
			switch arguments.first! {
			case "tag", "tags":
				removeNotificationTags()
			case "username", "usernames":
				removeNotificationUsernames()
			case "misleading", "misleadinglink", "misleadinglinks":
				removeNotificationLinks()
			default:
				reply("Unrecognized reason \"\(arguments.first!)\"; allowed reasons are `tags`, `usernames`, and `misleading links`.")
			}
		}
	}
}
