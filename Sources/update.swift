//
//  update.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

func launchProcess(path: String, arguments: [String]) -> Process {
	#if os(Linux)
		return Process.launchedTaskWithLaunchPath(path, arguments: arguments)
	#else
		return Process.launchedProcess(launchPath: path, arguments: arguments)
	#endif
}

var isUpdating = false

func installUpdate() -> Bool {
	do {
		#if os(Linux)
			let compile = "./build-nopm.sh"	//Swift Package Manager does not work on Raspberry Pi
			let move = "mv FireAlarm .."
		#else
			let compile = "./build.sh"
			let move = "mv ./.build/debug/FireAlarm .."
		#endif
		let updateScript = "pushd .;" +
			"(cd update && " +
			compile + "&& " +
			move + " && " +
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

private enum DownloadFailure: Error {
	case noAutoupdate
	case downloadFailed
}

var downloadedVersion: String = "<unknown>"
var availableVersion: String = "<unknown>"

func downloadUpdate(isAuto: Bool = false) throws {
	let script = "rm -rf update;" +
		"(git clone -b swift \"git://github.com/SOBotics/FireAlarm.git\" update && " +
		"cd update && " +
	"git log --format='oneline' -n 1 > ../version-downloaded.txt) || exit 1 "
	
	let process = launchProcess(path: "/bin/bash", arguments: ["-c", script])
	process.waitUntilExit()
	
	if process.terminationStatus != 0 {
		throw DownloadFailure.downloadFailed
	} else {
        downloadedVersion = availableVersion

		let downloaded = try loadFile("version-downloaded.txt")
		if isAuto && !downloaded.contains("--autoupdate") {
			throw DownloadFailure.noAutoupdate
		}
	}
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
		return "//github.com/SOBotics/FireAlarm/commit/\(version)"
	}
}


func prepareUpdate(_ listener: ChatListener, _ rooms: [ChatRoom], isAuto: Bool = false) -> Bool {
	if isUpdating {
		return true
	}
	
	do {
		try downloadUpdate(isAuto: isAuto)
	} catch DownloadFailure.noAutoupdate {
		return false
	} catch {
		handleError(error, "while downloading an update")
		return false
	}
	
	isUpdating = true
	rooms.forEach {$0.postMessage("Installing update..."); sleep(1)}
	listener.stop(.update)
	return true
}

func update(_ listener: ChatListener, _ rooms: [ChatRoom], force: Bool = false, auto: Bool = false) -> Bool {
	if force {
		return prepareUpdate(listener, rooms, isAuto: auto)
	}
	
	
	let versionScript = "git ls-remote git://github.com/SOBotics/FireAlarm swift | cut -d '\t' -f1 > available_version.txt"
	
	
	do {
		try versionScript.write(toFile: "get_version.sh", atomically: true, encoding: .utf8)
		let process = launchProcess(path: "/bin/bash", arguments: ["get_version.sh"])
		process.waitUntilExit()
		
		let versionContents = try loadFile("available_version.txt").replacingOccurrences(of: "\n", with: " ")
		let components = versionContents.components(separatedBy: " ")
        availableVersion = components.first ?? ""
		
		if currentVersion != availableVersion &&
            !(auto && downloadedVersion == availableVersion && downloadedVersion != "<unknown>") {
			return prepareUpdate(listener, rooms, isAuto: auto)
		}
	}
	catch {
		handleError(error, "while checking for updates")
	}
	return false
}
