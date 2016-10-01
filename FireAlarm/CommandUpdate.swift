//
//  CommandUpdate.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandUpdate: Command {
	override class func usage() -> [String] {
		return ["update"]
	}
	
	override func run() throws {
		if !update(bot) {
			bot.room.postReply("No new update available.", to: message)
		}
	}
}
