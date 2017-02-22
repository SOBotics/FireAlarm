//
//  CommandSetThreshold.swift
//  FireAlarm
//
//  Created by NobodyNada on 2/9/17.
//
//

import Foundation
import SwiftChatSE

class CommandSetThreshold: Command {
	override public class func usage() -> [String] {
		return ["set threshold *", "change threshold *", "change threshold to *", "set threshold to *"]
	}
	
	override public class func privileges() -> ChatUser.Privileges {
		return .filter
	}
	
	override func run() throws {
		guard let newThreshold = (arguments.first.map({Int($0)}) ?? nil) else {
			message.reply("Please specify a valid threshold.")
			return
		}
		
		message.room.threshold = newThreshold
		
		message.reply("The threshold for this room has been set to \(newThreshold) (*higher* thresholds report more posts).")
	}
}
