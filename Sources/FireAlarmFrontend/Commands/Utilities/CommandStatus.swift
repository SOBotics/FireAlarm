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
		return ["alive", "status", "version", "ver", "rev", "revision", "uptime"]
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
		if weeks != 0 { uptimeStrings.append("\(weeks) \(pluralize(weeks, "week"))") }
		if days != 0 { uptimeStrings.append("\(days) \(pluralize(days, "day"))") }
		if hours != 0 { uptimeStrings.append("\(hours) \(pluralize(hours, "hour"))") }
		if minutes != 0 { uptimeStrings.append("\(minutes) \(pluralize(minutes, "minute"))") }
		if seconds != 0 { uptimeStrings.append("\(seconds) \(pluralize(seconds, "second"))") }
		
		
		let status = "[\(botName)](\(stackAppsLink)) version [\(shortVersion)](\(versionLink)), " +
		"running for \(uptimeStrings.joined(separator: " ")) on \(location)"
		
		reply(status)
	}
}
