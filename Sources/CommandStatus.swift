//
//  CommandStatus.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/11/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import Dispatch
import SwiftChatSE

open class CommandStatus: Command {
	override open class func usage() -> [String] {
		return ["alive", "status", "version", "ver", "rev"]
	}
	
	override open func run() throws {
		var uptime = Int(Date().timeIntervalSinceReferenceDate - startTime.timeIntervalSinceReferenceDate)
		let seconds = uptime % 60
		uptime /= 60
		let minutes = uptime % 60
		uptime /= 60
		let hours = uptime % 24
		uptime /= 24
		let days = uptime % 7
		uptime /= 7
		let weeks = uptime
		
		var uptimeStrings: [String] = []
		if weeks != 0 {uptimeStrings.append("\(weeks) weeks")}
		if days != 0 {uptimeStrings.append("\(days) days")}
		if hours != 0 {uptimeStrings.append("\(hours) hours")}
		if minutes != 0 {uptimeStrings.append("\(minutes) minutes")}
		if seconds != 0 {uptimeStrings.append("\(seconds) seconds")}
		
		
		let status = "[\(botName)](\(stackAppsLink)) version [\(shortVersion)](\(versionLink)), " +
		"running for \(uptimeStrings.joined(separator: " ")) on \(location)"
		
		reply(status)
	}
}
