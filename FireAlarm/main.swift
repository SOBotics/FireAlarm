//
//  main.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/27/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

func clearCookies() {
	let storage = HTTPCookieStorage.shared
	if let cookies = storage.cookies {
		for cookie in cookies {
			storage.deleteCookie(cookie)
		}
	}
}



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



fileprivate var bot: ChatBot!

func main() throws {
	print("FireAlarm starting...")
	
	//Save the working directory & change to the chatbot directory.
	let originalWorkingDirectory = FileManager.default.currentDirectoryPath
	
	let saveDirURL = URL(fileURLWithPath: NSHomeDirectory()).appendingPathComponent(".firealarm", isDirectory: true)
	
	if !FileManager.default.fileExists(atPath: saveDirURL.path) {
		try! FileManager.default.createDirectory(at: saveDirURL, withIntermediateDirectories: false, attributes: nil)
	}
	
	saveURL = saveDirURL
	
	FileManager.default.changeCurrentDirectoryPath(saveDirURL.path)
	
	
	
	
	//Log in
	let client = Client(host: .StackOverflow)
	
	if !client.loggedIn {
		let email: String
		let password: String
		
		let env =  ProcessInfo.processInfo.environment
		
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
		catch Client.LoginError.loginFailed(let message) {
			print("Login failed: \(message)")
			exit(EXIT_FAILURE)
		}
		catch {
			print("Login failed with error \(error).\nClearing cookies and retrying.")
			clearCookies()
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
	let room = ChatRoom(client: client, roomID: 111347)  //SOBotics
	//let room = ChatRoom(client: client, roomID: 123602)  //FireAlarm Development
	try room.loadUserDB()
	errorRoom = room
	bot = ChatBot(room)
	room.delegate = bot
	try room.join()
	
	try bot.filter.start()
	
	
	//Startup finished
	if FileManager.default.fileExists(atPath: "update-failure") {
		room.postMessage("Update failed!")
	}
	else if let new = try? String(contentsOfFile: "version-new.txt") {
		room.postMessage("Updated from \(currentVersion) to \(new).")
		try! new.write(toFile: "version.txt", atomically: true, encoding: .utf8)
		currentVersion = new
		try! FileManager.default.removeItem(atPath: "version-new.txt")
	}
	else {
		room.postMessage("[FireAlarm-Swift](//github.com/NobodyNada/FireAlarm/tree/swift) started.")
	}
	
	
	
	//Run background tasks
	
	func autoUpdate() {
		sleep(60 * 15)
		//wait 15 minutes
		let _ = update(bot)
	}
	
	DispatchQueue.global().async { autoUpdate() }
	
	
	func inputMonitor() {
		repeat {
			if let input = readLine() {
				backgroundTasks.append(.handleInput(input: input))
				backgroundSemaphore.signal()
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
			
			
			try room.saveUserDB()
			
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
				FileManager.default.changeCurrentDirectoryPath(originalWorkingDirectory)
				
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

func handleError(_ error: Error, _ context: String? = nil) {
	let contextStr: String
	if context != nil {
		contextStr = " \(context!)"
	}
	else {
		contextStr = ""
	}
	
	let message1 = "    An error (\(String(reflecting: type(of: error)))) occured\(contextStr):"
	let message2 = String(describing: error)
	
	if let room = errorRoom {
		room.postMessage(message1 + "\n    " + message2.replacingOccurrences(of: "\n", with: "\n    "))
	}
	else {
		fatalError("\(message1)\n\(message2)")
	}
}



try! main()


