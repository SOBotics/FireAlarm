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
		return ["update force", "update"]
	}
	
	override func run() throws {
		if !update(bot, force: (usageIndex == 0)) {
			reply("No new update available.")
		}
	}
}
