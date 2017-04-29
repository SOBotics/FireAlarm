//
//  BackgroundTasks.swift
//  FireAlarm
//
//  Created by NobodyNada on 4/18/17.
//
//

import Foundation
import SwiftChatSE

func save(rooms: [ChatRoom]) {
	do {
		try rooms.forEach { try $0.saveUserDB() }
	} catch {
		handleError(error, "while saving the user database")
	}
	do {
		try filter.filterBlacklistedUsernames.saveUsernameBlacklist()
	} catch {
		handleError(error, "while saving the username blacklist")
	}
	do {
		try filter.reports.saveReports()
	} catch {
		handleError(error, "while saving reports")
	}
	
	do {
		try redunda?.uploadFiles()
	} catch {
		print("Could not upload files!")
		print(error)
	}
}


func handleInput(input: String, rooms: [ChatRoom], listener: ChatListener) {
	var messageContent = input
	
	guard let firstComponent = input.components(separatedBy: .whitespaces).first else {
		return
	}
	
	let room: ChatRoom
	if firstComponent.hasPrefix(">") {
		let roomIDStr = firstComponent.substring(
			from: firstComponent.characters.index(after: firstComponent.characters.startIndex)
		)
		guard let roomID = Int(roomIDStr) else {
			print("Invalid room ID.")
			return
		}
		
		guard let roomIndex = rooms.index(where: { roomID == $0.roomID }) else {
			print("I'm not  in that room.")
			return
		}
		
		room = rooms[roomIndex]
		
		messageContent = input.components(separatedBy: .whitespaces).dropFirst().joined(separator: " ")
	} else {
		room = rooms.first!
	}
	
	listener.processMessage(
		room,
		message: ChatMessage(
			room: room,
			user: room.userWithID(0),
			content: messageContent,
			id: nil
		),
		isEdit: false
	)
}


func shutDown(reason: ChatListener.StopReason, rooms: [ChatRoom]) {
	var shouldReboot = reason == .reboot
	
	filter.postFetcher.stop()
	
	//Wait for pending messages to be posted.
	for room in rooms {
		while !room.messageQueue.isEmpty {
			sleep(1)
		}
	}
	while filter != nil && !(filter.postFetcher.ws.state == .disconnected || filter.postFetcher.ws.state == .error) {
		sleep(1)
	}
	
	save(rooms: rooms)
	
	if reason == .update {
		if installUpdate() {
			execv(saveDirURL.appendingPathComponent("firealarm").path, CommandLine.unsafeArgv)
		}
		else {
			shouldReboot = true
		}
	}
	
	rooms.forEach { $0.leave() }
	
	if shouldReboot {
		//Change to the old working directory.
		let _ = FileManager.default.changeCurrentDirectoryPath(originalWorkingDirectory)
		
		//Reload the program binary, which will restart the bot.
		execv(CommandLine.arguments[0], CommandLine.unsafeArgv)
		//If the exec failed, exit 1 for my script
		//which automatically reboots on crashes.
		exit(1)
	}
	
	exit(0)
}




func scheduleBackgroundTasks(rooms: [ChatRoom], listener: ChatListener) {
	BackgroundTaskManager.shared.tasks = [
		//Save
		BackgroundTask(interval: 60) {task in
			save(rooms: rooms)
		},
		
		
		//Update
		BackgroundTask(interval: 60) {task in
			if development || noUpdate || update(listener, [rooms.first!], auto: true) {
				task.cancel()
			}
		},
		
		
		//Watch for input
		BackgroundTask() { task in
			repeat {
				if let input = readLine() {
					handleInput(input: input, rooms: rooms, listener: listener)
				} else {
					//if EOF is reached,
					return
				}
			} while !task.isCancelled
		},
		
		BackgroundTask(interval: 1) { task in
			do {
				let manager = FileManager.default
				let file = "input.txt"
				if manager.fileExists(atPath: file) {
					let input = String(
						data: try Data(contentsOf: saveURL.appendingPathComponent(file)),
						encoding: .utf8
						)!.trimmingCharacters(in: .whitespacesAndNewlines)
					
					try manager.removeItem(atPath: file)
					
					handleInput(input: input, rooms: rooms, listener: listener)
				}
			} catch {
				handleError(error, "while monitoring input.txt")
			}
		},
		
		
		//Ping Redunda
		BackgroundTask(interval: 30) {task in
			guard let r = redunda else { task.cancel(); return }
			do {
				if getShortVersion(currentVersion) == "<unknown>" {
					try r.sendStatusPing()
				} else {
					try r.sendStatusPing(version: getShortVersion(currentVersion))
				}
				
				if r.shouldStandby {
					rooms.first!.postMessage("[ [\(botName)](\(githubLink)) ] Switching to standby mode on \(location).")
					
					shutDown(reason: .reboot, rooms: rooms)
				}
			} catch {
				handleError(error, "while sending a status ping to Redunda")
			}
		}
	]
}
