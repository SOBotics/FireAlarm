//
//  update.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

#if os(Linux)
	typealias Process = Task
#endif

func launchProcess(path: String, arguments: [String]) -> Process {
	#if os(Linux)
		return Process.launchedTaskWithLaunchPath(path, arguments: arguments)
	#else
		return Process.launchedProcess(launchPath: path, arguments: arguments)
	#endif
}

func installUpdate() -> Bool {
	do {
		#if os(Linux)
			let compile = "./build-nopm.sh"	//Swift Package Manager does not work on Raspberry Pi
		#else
			let compile = "./build.sh"
		#endif
		let updateScript = "rm -rf update;pushd .;" +
			"(git clone -b swift \"git://github.com/NobodyNada/FireAlarm.git\" update && " +
			"cd update && " +
			compile + "&& " +
			"mv FireAlarm .. && " +
			"git log --format='oneline' -n 1 > ../version-new.txt && " +
			"cd .. && " +
			"rm -rf update) || " +
			"(popd; " +
		"touch update-failure)"
		
		if FileManager.default.fileExists(atPath: "update-failure") {
			try FileManager.default.removeItem(atPath: "update-failure")
		}
		
		try updateScript.write(toFile: "update.sh", atomically: true, encoding: .utf8)
		
		let process = launchProcess(path: "/bin/bash", arguments: ["update.sh"])
		process.waitUntilExit()
		
		
		return !FileManager.default.fileExists(atPath: "update-failure")
		
	} catch {
		handleError(error, "while installing an update")
	}
	
	return false
}

func getCurrentVersion() -> String {
	return (try? loadFile("version.txt").replacingOccurrences(of: "\n", with: "")) ?? "<unknown version>"
}

public func getShortVersion(_ version: String) -> String {
	if version == "<unknown version>" {
		return "<unknown>"
	}
	return version.characters.count > 7 ?
		String(version.characters[version.characters.startIndex..<version.characters.index(version.characters.startIndex, offsetBy: 7)]) :
	"<unknown version>"
}

public func getVersionLink(_ version: String) -> String {
	if version == "<unknown version>" {
		return githubLink
	} else {
		return "//github.com/NobodyNada/FireAlarm/commit/\(version)"
	}
}


func prepareUpdate(_ listener: ChatListener, _ room: ChatRoom) {
	room.postMessage("Installing update...")
	listener.stop(.update)
}

func update(_ listener: ChatListener, _ room: ChatRoom, force: Bool = false) -> Bool {
	if force {
		prepareUpdate(listener, room)
		return true
	}
	
	
	let versionScript = "git ls-remote git://github.com/NobodyNada/FireAlarm swift | cut -d '\t' -f1 > available_version.txt"
	
	
	
	do {
		
		try versionScript.write(toFile: "get_version.sh", atomically: true, encoding: .utf8)
		let process = launchProcess(path: "/bin/bash", arguments: ["get_version.sh"])
		process.waitUntilExit()
		
		let versionContents = try loadFile("available_version.txt").replacingOccurrences(of: "\n", with: " ")
		let components = versionContents.components(separatedBy: " ")
		let availableVersion = components.first ?? ""
		
		if currentVersion != availableVersion {
			prepareUpdate(listener, room)
			return true
		}
	}
	catch {
		handleError(error, "while checking for updates")
	}
	return false
}
