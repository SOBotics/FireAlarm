//
//  CommandUpdate.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Cocoa

class CommandUpdate: Command {
	override class func usage() -> [String] {
		return ["update"]
	}
	
	override func run() throws {
		update(bot)
	}
}
