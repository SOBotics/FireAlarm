//
//  startup.swift
//  FireAlarm
//
//  Created by NobodyNada on 4/18/17.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import Dispatch
import SwiftChatSE
import SwiftStack

let commands: [Command.Type] = [
	CommandSay.self,
	CommandHelp.self, CommandListRunning.self, CommandStop.self, CommandKill.self, CommandUpdate.self, CommandStatus.self,
	CommandCheckThreshold.self, CommandSetThreshold.self,
	CommandCheckPrivileges.self, CommandPrivilege.self, CommandUnprivilege.self,
	CommandCheckPost.self, CommandQuota.self,
	CommandBlacklistUsername.self, CommandGetBlacklistedUsernames.self, CommandUnblacklistUsername.self,
	CommandOptIn.self, CommandOptOut.self, CommandCheckNotification.self, CommandLeaveRoom.self,
	CommandLocation.self, CommandReport.self, CommandUnclosed.self, CommandTestBayesian.self,
]



fileprivate var listener: ChatListener!

var filter: Filter!

var redunda: Redunda?

//var apiClient = APIClient(proxyAddress: "127.0.0.1", proxyPort: 8080)
var apiClient = APIClient()

func main() throws {
	print("FireAlarm starting...")
	startTime = Date()
	afterTooManyErrors = {}
	addPrivileges()
	
	
	//Save the working directory & change to the chatbot directory.
	originalWorkingDirectory = FileManager.default.currentDirectoryPath
	
	let saveDirURL = URL(fileURLWithPath: NSHomeDirectory()).appendingPathComponent(".firealarm", isDirectory: true)
	
	if !FileManager.default.fileExists(atPath: saveDirURL.path) {
		try! FileManager.default.createDirectory(at: saveDirURL, withIntermediateDirectories: false, attributes: nil)
	}
	
	saveURL = saveDirURL
	
	let _ = FileManager.default.changeCurrentDirectoryPath(saveDirURL.path)
	
	
	
	apiClient.key = "HNA2dbrFtyTZxeHN6rThNg(("
	apiClient.defaultFilter = "!-*f(6rOFHc24"

	let client = Client(host: .StackOverflow)

	
	
	if let redundaKey = try? loadFile("redunda_key.txt").trimmingCharacters(in: .whitespacesAndNewlines) {
		//standby until Redunda tells us not to
		redunda = Redunda(key: redundaKey, client: client, filesToSync: [
			"^filter\\.json$", "^reports\\.json$", "^room_\\d+_[a-z\\.]+\\.json$"
		])
		
		var shouldStandby = false
		var isFirst = true
		repeat {
			do {
				try redunda!.sendStatusPing()
				if redunda!.shouldStandby {
					shouldStandby = true
					if isFirst {
						print("FireAlarm started in standby mode.")
					}
					isFirst = false
					sleep(30)
				} else {
					shouldStandby = false
				}
			} catch {
				handleError(error, "while sending a status ping to Redunda")
			}
		} while shouldStandby
		if !isFirst {
			print("FireAlarm activating...")
		}
		
		do {
			try redunda!.downloadFiles()
		} catch {
			print("Could not download files!")
		}
	}
	
	
	
	//Log in
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
			try client.login(email: email, password: password)
		}
		catch {
			handleError(error, "while logging in")
			exit(EXIT_FAILURE)
		}
	}
	
	
	
    //Get the location
    var rawLocation = ""
    do {
        rawLocation = try loadFile ("location.txt")
        let components = rawLocation.components(separatedBy: CharacterSet.whitespaces)
        
        user = components.first ?? ""
        device = components.dropFirst().joined(separator: " ")
        
        location = "\(user)/\(device)"
        
        location = String(location.characters.filter { !"\n".characters.contains($0) })
        userLocation = location
        user = String(user.characters.filter { !"\n".characters.contains($0) })
        device = String(device.characters.filter { !"\n".characters.contains($0) })
        ping = " (cc @\(user))"
		userLocation = location
    } catch {
        print ("Location could not be loaded!")
    }
	
	
	
	//Join the chat room
	let rooms: [ChatRoom]
	if let devString = env["DEVELOPMENT"], let devRoom = Int(devString) {
		rooms = [ChatRoom(client: client, roomID: devRoom)]
		development = true
	}
	else {
		rooms = [
			ChatRoom(client: client, roomID: 123602), //FireAlarm Development
			ChatRoom(client: client, roomID: 111347), //SOBotics
			ChatRoom(client: client, roomID: 41570),  //SO Close Vote Reviewers
		]
		
		development = false
	}
	try rooms.forEach {try $0.loadUserDB()}
	
	afterTooManyErrors = {
		print("Too many errors; aborting...")
		abort()
	}
	errorRoom = rooms.first!
	
	
	
	
	listener = ChatListener(commands: commands)
	listener.onShutdown { shutDown(reason: $0, rooms: rooms) }
	rooms.forEach {room in room.onMessage { listener.processMessage(room, message: $0, isEdit: $1) } }
	
	try rooms.forEach { try $0.join() }
	
	//Post the startup message
	let startupMessage: String?
	let startupMessageCompletion: ((Int) -> Void)?
	
	currentVersion = getCurrentVersion()
	if FileManager.default.fileExists(atPath: "update-failure") {
		startupMessage = "Update failed!"
		startupMessageCompletion = {_ in
			do {
				try FileManager.default.removeItem(atPath: "update-failure")
			} catch {
				handleError(error, "while clearing the update status")
			}
		}
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
		
		startupMessage = "Updated from [`\(oldShort)`](\(oldLink)) to [`\(newShort)`](\(newLink))\(message)."		
		try! new.write(toFile: "version.txt", atomically: true, encoding: .utf8)
		currentVersion = new
		startupMessageCompletion = {_ in
			do {
				if FileManager.default.fileExists(atPath: "version-new.txt") {
					try FileManager.default.removeItem(atPath: "version-new.txt")
				}
			} catch {
				handleError(error, "while clearing the update status")
			}
		}
	}
	else {
		startupMessage = nil
		startupMessageCompletion = nil
		let short = getShortVersion(currentVersion)
		let link = getVersionLink(currentVersion)
		
		rooms.first?.postMessage("[ [\(botName)](\(stackAppsLink)) ] FireAlarm started at revision [`\(short)`](\(link)) on \(location).")
	}
	
	if let message = startupMessage {
		//rooms.dropLast().forEach { $0.postMessage(message); sleep(1) }
		//rooms.last!.postMessage(message, completion: startupMessageCompletion);
		rooms.first?.postMessage(message, completion: startupMessageCompletion)
	}
	
	shortVersion = getShortVersion(currentVersion)
	versionLink = getVersionLink(currentVersion)
	
	
	
	
	//Load the filter
	filter = Filter(rooms)
	try filter.postFetcher.start()
	
	errorsInLast30Seconds = 0
	afterTooManyErrors = {
		print("Too many errors; aborting...")
		abort()
	}
	
	
	scheduleBackgroundTasks(rooms: rooms, listener: listener)
}

//func halt(reboot: Bool = false, update: Bool = false) {
//	shutDown(update: update, reboot: reboot, rooms: rooms)
//	backgroundTasks.append(.shutDown(reboot: reboot, update: update))
//	backgroundSemaphore.signal()
//}


