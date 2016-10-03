//
//  update.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

#if os(Linux)
	
	func launchProcess(path: String, arguments: [String]) -> Task {
		return Task.launchedTaskWithLaunchPath(path, arguments: arguments)
	}
	
#else
	
	func launchProcess(path: String, arguments: [String]) -> Process {
		return Process.launchedProcess(launchPath: path, arguments: arguments)
	}
	
#endif

func installUpdate() -> Bool {
	do {
		let updateScript = "rm -rf update;pushd .;" +
			"(git clone -b swift \"https://github.com/NobodyNada/FireAlarm.git\" update && " +
			"cd update && " +
			"swiftc -sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk " +
			"-target x86_64-macosx10.11 -lz -lc++ -o ../FireAlarm FireAlarm/*.swift && " +
			"git log --pretty=format:'%h' -n 1 > ../version-new.txt && " +
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

var currentVersion = (try? String(contentsOfFile: "version.txt").replacingOccurrences(of: "\n", with: "")) ?? ""

func prepareUpdate(_ bot: ChatBot) {
	bot.room.postMessage("Installing update...")
	bot.stop(.update)
}

func update(_ bot: ChatBot) -> Bool {
	
	
	let versionScript = "git ls-remote https://github.com/NobodyNada/FireAlarm swift | cut -c1-7 > available_version.txt"
	
	
	
	do {
		
		try versionScript.write(toFile: "get_version.sh", atomically: true, encoding: .utf8)
		let process = launchProcess(path: "/bin/bash", arguments: ["get_version.sh"])
		process.waitUntilExit()
		
		let availableVersion = try String(contentsOfFile: "available_version.txt").replacingOccurrences(of: "\n", with: "")
		
		if currentVersion != availableVersion {
			prepareUpdate(bot)
			return true
		}
	}
	catch {
		handleError(error, "while checking for updates")
	}
	return false
}
