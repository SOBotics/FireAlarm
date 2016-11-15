//
//  update.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

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
			let compile = "swiftc -l:libz.a -o ../FireAlarm FireAlarm/*.swift"
		#else
			let compile = "swiftc -sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk " +
			"-target x86_64-macosx10.11 -lz -lc++ -o ../FireAlarm FireAlarm/*.swift "
		#endif
		let updateScript = "rm -rf update;pushd .;" +
			"(git clone -b swift \"https://github.com/NobodyNada/FireAlarm.git\" update && " +
			"cd update && " +
			compile + "&& " +
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

var _version: String? = (try? String(contentsOfFile: "version.txt").replacingOccurrences(of: "\n", with: ""))

var currentVersion: String {
get {
	return _version ?? ""
}
set {
	_version = newValue
}
}

public func getShortVersion(_ version: String) -> String {
	return version.characters.count > 8 ?
		String(version.characters[version.characters.startIndex..<version.characters.index(version.characters.startIndex, offsetBy: 8)]) :
	"<unknown version>"
}

public func getVersionLink(_ version: String) -> String {
	return "//github.com/NobodyNada/FireAlarm/commit/\(version)"
}

public var version: String {
	return _version ?? "<unknown version>"
}

public

func prepareUpdate(_ bot: ChatBot) {
	bot.room.postMessage("Installing update...")
	bot.stop(.update)
}

func update(_ bot: ChatBot) -> Bool {
	
	
	let versionScript = "git ls-remote https://github.com/NobodyNada/FireAlarm swift | cut -d '\t' -f1 > available_version.txt"
	
	
	
	do {
		
		try versionScript.write(toFile: "get_version.sh", atomically: true, encoding: .utf8)
		let process = launchProcess(path: "/bin/bash", arguments: ["get_version.sh"])
		process.waitUntilExit()
		
		let versionContents = try String(contentsOfFile: "available_version.txt").replacingOccurrences(of: "\n", with: " ")
		let components = versionContents.components(separatedBy: " ")
		let availableVersion = components.first ?? ""
		
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
