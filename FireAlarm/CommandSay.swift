//
//  CommandSay.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandSay: Command {
	override class func usage() -> [String] {
		return ["say ..."]
	}
	
	override func run() throws {
		message.room.postMessage(message.content.components(separatedBy: " ").dropFirst().dropFirst().joined(separator: " "))
	}
}
