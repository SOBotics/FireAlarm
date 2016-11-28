//
//  main.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/27/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import Dispatch

let commands: [Command.Type] = [
	CommandSay.self,
	CommandHelp.self, CommandListRunning.self, CommandStop.self, CommandKill.self, CommandUpdate.self, CommandStatus.self,
	CommandCheckPost.self,
	CommandOptIn.self, CommandOptOut.self, CommandCheckNotification.self,
	CommandCheckPrivileges.self, CommandPrivilege.self, CommandUnprivilege.self,
]


extension ChatUser {
	var notified: Bool {
		get {
			return ((info["notified"] as? Int) ?? 0) == 1 ? true : false
		} set {
			info["notified"] = (newValue ? 1 : 0)
		}
	}
	var notificationTags: [String] {
		get {
			return (info["notificationTags"] as? [String]) ?? []
		} set {
			info["notificationTags"] = newValue
		}
	}
}


private enum BackgroundTask {
	case handleInput(input: String)
	case shutDown(reboot: Bool, update: Bool)
}

private var backgroundTasks = [BackgroundTask]()
private let backgroundSemaphore = DispatchSemaphore(value: 0)

private var saveURL: URL!

enum SaveFileAccessType {
	case reading
	case writing
	case updating
}

func saveFileNamed(_ name: String) -> URL {
	return saveURL.appendingPathComponent(name)
}

let saveDirURL = URL(fileURLWithPath: NSHomeDirectory()).appendingPathComponent(".firealarm", isDirectory: true)


public var startTime: Date!

fileprivate var listener: ChatListener!

func main() throws {
	print("FireAlarm starting...")
	
	startTime = Date()
	
	//Save the working directory & change to the chatbot directory.
	let originalWorkingDirectory = FileManager.default.currentDirectoryPath
	
	let saveDirURL = URL(fileURLWithPath: NSHomeDirectory()).appendingPathComponent(".firealarm", isDirectory: true)
	
	if !FileManager.default.fileExists(atPath: saveDirURL.path) {
		try! FileManager.default.createDirectory(at: saveDirURL, withIntermediateDirectories: false, attributes: nil)
	}
	
	saveURL = saveDirURL
	
	let _ = FileManager.default.changeCurrentDirectoryPath(saveDirURL.path)
	
	
	
	
	//Log in
	let client = Client(host: .StackOverflow)
	
	let env =  ProcessInfo.processInfo.environment
	
	if !client.loggedIn {
		let email: String
		let password: String
		
		let envEmail = env["ChatBotEmail"]
		let envPassword = env["ChatBotPass"]
		
		if envEmail != nil {
			email = envEmail!
		}
		else {
			print("Email: ", terminator: "")
			email = readLine()!
		}
		
		if envPassword != nil {
			password = envPassword!
		}
		else {
			password = String(validatingUTF8: getpass("Password: "))!
		}
		
		do {
			try client.loginWithEmail(email, password: password)
		}
		catch {
			handleError(error, "while logging in")
			exit(EXIT_FAILURE)
		}
	}
	
	
	
	//Join the chat room
	let room: ChatRoom
	let development: Bool
	if let devString = env["DEVELOPMENT"], let devRoom = Int(devString) {
		room = ChatRoom(client: client, roomID: devRoom)
		development = true
	}
	else {
		room = ChatRoom(client: client, roomID: 111347)  //SOBotics
		development = false
	}
	try room.loadUserDB()
	errorRoom = room
	listener = ChatListener(room, commands: commands)
	room.delegate = listener
	try room.join()
	
	try listener.filter.start()
	
	
	//Startup finished
	if FileManager.default.fileExists(atPath: "update-failure") {
		room.postMessage("Update failed!")
		try! FileManager.default.removeItem(atPath: "update-failure")
	}
	else if let new = try? loadFile("version-new.txt").replacingOccurrences(of: "\n", with: "") {
		let components = new.components(separatedBy: " ")
		let new = components.first ?? ""
		let newShort = getShortVersion(new)
		let newLink = getVersionLink(new)
		
		let old = currentVersion
		let oldShort = getShortVersion(old)
		let oldLink = getVersionLink(old)
		
		let message = components.count > 1 ? (" (" + components[1..<components.count].joined(separator: " ") + ")") : ""
		
		room.postMessage("Updated from [`\(oldShort)`](\(oldLink)) to [`\(newShort)`](\(newLink))\(message).")
		
		try! new.write(toFile: "version.txt", atomically: true, encoding: .utf8)
		currentVersion = new
		try! FileManager.default.removeItem(atPath: "version-new.txt")
	}
	else {
		room.postMessage("[\(botName)](\(githubLink)) started.")
	}
	
	
	
	//Run background tasks
	
	func autosaveAndUpdate() {
		var updated = false
		while true {
			//wait one minute
			sleep(60)
			if !updated && !development {
				updated = update(listener)
			}
			
			do {
				try room.saveUserDB()
			} catch {
				handleError(error, "while saving the user database")
			}
		}
	}
	
	DispatchQueue.global().async { autosaveAndUpdate() }
	
	
	func inputMonitor() {
		repeat {
			if let input = readLine() {
				backgroundTasks.append(.handleInput(input: input))
				backgroundSemaphore.signal()
			} else {
				//if EOF is reached,
				return
			}
		} while true
	}
	
	
	DispatchQueue.global().async(execute: inputMonitor)
	
	
	repeat {
		//wait for a background task
		backgroundSemaphore.wait()
		
		switch backgroundTasks.removeFirst() {
		case .handleInput(let input):
			listener.chatRoomMessage(
				room,
				message: ChatMessage(
					room: room,
					user: room.userWithID(0),
					content: input,
					id: nil
				),
				isEdit: false
			)
		case .shutDown(let reboot, let update):
			var shouldReboot = reboot
			//Wait for pending messages to be posted.
			while !room.messageQueue.isEmpty {
				sleep(1)
			}
			
			do {
				try room.saveUserDB()
			} catch {
				handleError(error, "while saving the user database")
			}
			
			if update {
				if installUpdate() {
					execv(saveDirURL.appendingPathComponent("firealarm").path, CommandLine.unsafeArgv)
				}
				else {
					shouldReboot = true
				}
			}
			
			room.leave()
			
			if shouldReboot {
				//Change to the old working directory.
				let _ = FileManager.default.changeCurrentDirectoryPath(originalWorkingDirectory)
				
				//Reload the program binary, which will restart the bot.
				execv(CommandLine.arguments[0], CommandLine.unsafeArgv)
			}
			//If a reboot fails, it will fall through to here & just shutdown instead.
			return
		}
	} while true
}

func halt(reboot: Bool = false, update: Bool = false) {
	backgroundTasks.append(.shutDown(reboot: reboot, update: update))
	backgroundSemaphore.signal()
}




try! main()


