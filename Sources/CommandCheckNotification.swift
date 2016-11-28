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
			reply("You will not notified of any reports.")
		}
		else if message.user.notificationTags.isEmpty {
			reply("You will be notified of all reports.")
		}
		else {
			let formatted = formatArray(message.user.notificationTags.map { "[tag:\($0)]" }, conjunction: "or")
			reply("You will be notified of reports tagged \(formatted).")
		}

	}
}
