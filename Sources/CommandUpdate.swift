//
//  CommandUpdate.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandUpdate: Command {
	override class func usage() -> [String] {
		return ["update force", "update", "pull"]
	}
	
	override class func privileges() -> ChatUser.Privileges {
		return .owner
	}
	
	override func run() throws {
		if !update(listener, [message.room], force: (usageIndex == 0), auto: true) {
			reply("No new update available.")
		}
	}
}
