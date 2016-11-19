//
//  CommandStatus.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 10/11/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import Dispatch

class CommandStatus: Command {
	override class func usage() -> [String] {
		return ["alive", "status", "version"]
	}
	
	override func run() throws {
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
		
		let pipe = Pipe()
		let process = Process()
		process.launchPath = "/usr/bin/env"
		process.arguments = ["uname", "-srm"]
		process.standardOutput = pipe
		process.launch()
		
		var machineInfo: String = "<failed to get machine information>"
		
		let sema = DispatchSemaphore(value: 0)
		
		process.terminationHandler = {process in
			if let info = String(data: pipe.fileHandleForReading.readDataToEndOfFile(), encoding: .utf8) {
				machineInfo = info.trimmingCharacters(in: .whitespacesAndNewlines)
			}
			sema.signal()
		}
		
		let _ = sema.wait(timeout: DispatchTime.now() + DispatchTimeInterval.seconds(5))
		
		let shortVersion = getShortVersion(version)
		let link = getVersionLink(version)
		
		let status = "[FireAlarm-Swift](\(githubLink)) version [\(shortVersion)](\(link)), " +
		"running for \(uptimeStrings.joined(separator: " ")) on \(machineInfo)"
		
		bot.room.postReply(status, to: message)
	}
}
