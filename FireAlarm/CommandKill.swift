//
//  CommandKill.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Cocoa

class CommandKill: Command {
	override class func usage() -> [String] {
		return ["kill"]
	}
	
	override func run() throws {
		abort()
	}
}
