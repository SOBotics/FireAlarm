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
	
	override func run() throws {
		message.user.notified = true
		
		for tag in arguments {
			if !message.user.notificationTags.contains(tag) {
				message.user.notificationTags.append(tag)
			}
		}
		
		if arguments.count == 0 {
			message.user.notificationTags = []
			reply("You will now be notified of all reports.")
		}
		else {
			let formatted = formatArray(message.user.notificationTags.map { "[tag:\($0)]" }, conjunction: "or")
			reply("You will now be notified of reports tagged \(formatted).")
		}
	}
}
