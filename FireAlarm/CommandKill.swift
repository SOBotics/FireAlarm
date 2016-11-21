//
//  CommandKill.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandKill: Command {
	override class func usage() -> [String] {
		return ["kill"]
	}
	
	override class func privileges() -> ChatUser.Privileges {
		return [.owner]
	}
	
	override func run() throws {
		abort()
	}
}
