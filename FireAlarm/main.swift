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
	CommandTest.self, CommandSay.self,
	CommandHelp.self, CommandListRunning.self, CommandStop.self, CommandKill.self, CommandUpdate.self, CommandStatus.self,
	CommandCheckPost.self,
	CommandOptIn.self, CommandOptOut.self, CommandCheckNotification.self,
	CommandCheckPrivileges.self, CommandPrivilege.self, CommandUnprivilege.self,
]

enum FileLoadingError: Error {
	case notUFT8
}

func loadFile(_ path: String) throws -> String {
	let data = try Data(contentsOf: URL(fileURLWithPath: path))
	guard let str = String(data: data, encoding: .utf8) else {
		throw FileLoadingError.notUFT8
	}
	
	return str
}

func formatArray<T>(_ array: [T], conjunction: String) -> String {
	var string = ""
	if array.count == 1 {
		string = "\(array.first!)"
	}
	else {
		for (index, item) in array.enumerated() {
			if index == array.count - 1 {
				string.append("\(conjunction) \(item)")
			}
			else {
				string.append("\(item)\(array.count == 2 ? "" : ",") ")
			}
		}
	}
	return string
}

func pluralize(_ n: Int, _ singular: String, _ plural: String? = nil) -> String {
	let resultPlural: String
	if let p = plural {
		resultPlural = p
	} else {
		resultPlural = singular + "s"
	}
	
	if n == 1 {
		return singular
	} else {
		return resultPlural
	}
}


#if os(macOS)
	func clearCookies(_ storage: HTTPCookieStorage) {
		if let cookies = storage.cookies {
			for cookie in cookies {
				storage.deleteCookie(cookie)
			}
		}
	}
#endif

public var githubLink = "//github.com/NobodyNada/FireAlarm/tree/swift"

func makeTable(_ heading: [String], contents: [String]...) -> String {
	if heading.count != contents.count {
		fatalError("heading and contents have different counts")
	}
	let cols = heading.count
	
	var alignedHeading = [String]()
	var alignedContents = [[String]]()
	
	var maxLength = [Int]()
	
	var rows = 0
	var tableWidth = 0
	
	for col in 0..<cols {
		maxLength.append(heading[col].characters.count)
		for row in contents[col] {
			maxLength[col] = max(row.characters.count, maxLength[col])
		}
		rows = max(contents[col].count, rows)
		alignedHeading.append(heading[col].padding(toLength: maxLength[col], withPad: " ", startingAt: 0))
		alignedContents.append(contents[col].map {
			$0.padding(toLength: maxLength[col], withPad: " ", startingAt: 0)
			}
		)
		tableWidth += maxLength[col]
	}
	tableWidth += (cols - 1) * 3
	
	let head = alignedHeading.joined(separator: " | ")
	let divider = String([Character](repeating: "-", count: tableWidth))
	var table = [String]()
	
	for row in 0..<rows {
		var columns = [String]()
		for col in 0..<cols {
			columns.append(
				alignedContents[col].count > row ?
					alignedContents[col][row] : String([Character](repeating: " ", count: maxLength[col])))
		}
		table.append(columns.joined(separator: " | "))
	}
	
	return "    " + [head,divider,table.joined(separator: "\n    ")].joined(separator: "\n    ")
}

#if os(Linux)
	extension ChatUser {
		var notified: Bool {
			get {
				return (info["notified"] as? Bool) ?? false
			} set {
				info["notified"] = newValue._bridgeToObjectiveC()
			}
		}
		var notificationTags: [String] {
			get {
				return (info["notificationTags"] as? [String]) ?? []
			} set {
				info["notificationTags"] = newValue._bridgeToObjectiveC()
			}
		}
	}
#else
	extension ChatUser {
		var notified: Bool {
			get {
				return (info["notified"] as? Bool) ?? false
			} set {
				info["notified"] = newValue as AnyObject
			}
		}
		var notificationTags: [String] {
			get {
				return (info["notificationTags"] as? [String]) ?? []
			} set {
				info["notificationTags"] = newValue as AnyObject
			}
		}
	}
#endif


private var errorRoom: ChatRoom?
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

fileprivate var bot: ChatBot!

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
			print("Login failed with error \(error).\nClearing cookies and retrying.")
			//#if os(macOS)
			//clearCookies(client.cookieStorage)
			//#endif
			do {
				try client.loginWithEmail(email, password: password)
			}
			catch {
				print("Failed to log in!")
				exit(EXIT_FAILURE)
			}
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
	bot = ChatBot(room, commands: commands)
	room.delegate = bot
	try room.join()
	
	try bot.filter.start()
	
	
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
		room.postMessage("[FireAlarm-Swift](\(githubLink)) started.")
	}
	
	
	
	//Run background tasks
	
	func autoUpdate() {
		var updated = false
		while !updated {
			sleep(60)
			//wait one minute
			updated = update(bot)
		}
	}
	
	if !development {
		DispatchQueue.global().async { autoUpdate() }
	}
	
	
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
			bot.chatRoomMessage(
				room,
				message: ChatMessage(
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
			room.leave()
			
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

func handleError(_ error: Any, _ context: String? = nil) {
	let contextStr: String
	let errorType: String
	let errorDetails: String
	
	#if os(Linux)
		if type(of: error) == NSError.self {
			errorType = "NSError"
			errorDetails = unsafeBitCast(error, to: NSError.self).localizedDescription
		} else {
			errorType = String(reflecting: type(of: error))
			errorDetails = String(describing: error)
		}
	#else
		errorType = String(reflecting: type(of: error))
		errorDetails = String(describing: error)
	#endif
	
	if context != nil {
		contextStr = " \(context!)"
	}
	else {
		contextStr = ""
	}
	
	let message1 = "    An error (\(errorType)) occured\(contextStr):"
	
	if let room = errorRoom {
		room.postMessage(message1 + "\n    " + errorDetails.replacingOccurrences(of: "\n", with: "\n    "))
	}
	else {
		print("\(message1)\n\(errorDetails)")
		exit(1)
	}
}



try! main()


