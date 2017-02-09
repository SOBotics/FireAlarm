//
//  CommandCheckThreshold.swift
//  FireAlarm
//
//  Created by NobodyNada on 2/9/17.
//
//

import Foundation
import SwiftChatSE

class CommandCheckThreshold: Command {
	override class func usage() -> [String] {
		return ["threshold", "check threshold", "get threshold", "current threshold"]
	}
	
	override func run() throws {
		message.room.postReply("The threshold for this room is \(message.room.threshold) (*higher* thresholds report more posts).", to: message)
	}
}
