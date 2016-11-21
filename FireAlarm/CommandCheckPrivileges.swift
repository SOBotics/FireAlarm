//
//  CheckPrivileges.swift
//  FireAlarm
//
//  Created by NobodyNada on 11/20/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandCheckPrivileges: Command {
	override class func usage() -> [String] {
		return ["privileges", "amiprivileged", "am i privileged", "my privileges", "check privileges"]
	}
	
	
	override func run() throws {
		let privs = message.user.privileges.names
		if privs.isEmpty {
			reply("You do not have any privileges.")
		} else {
			reply("You have the \(formatArray(privs, conjunction: "and")) \(pluralize(privs.count, "privilege")).")
		}
	}
}
