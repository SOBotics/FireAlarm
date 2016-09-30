//
//  ChatBot.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class ChatBot: ChatRoomDelegate {
	let room: ChatRoom
	
	let commands: [Command.Type] = [
		CommandTest.self,
		CommandHelp.self, CommandListRunning.self, CommandStop.self, CommandUpdate.self
	]
	
	var filter: Filter!
	
	let commandQueue = DispatchQueue(label: "Command Queue", attributes: DispatchQueue.Attributes.concurrent)
	
	var runningCommands = [Command]()
	
	enum StopAction {
		case run
		case halt
		case reboot
		case update
	}
	
	fileprivate var pendingStopAction = StopAction.run
	
	fileprivate func runCommand(_ command: Command) {
		runningCommands.append(command)
		commandQueue.async {
			do {
				try command.run()
			}
			catch {
				handleError(error, "while running \"\(command.message.content)\"")
			}
			self.runningCommands.remove(at: self.runningCommands.index {$0 === command}!)
			if (self.pendingStopAction != .run) && self.runningCommands.isEmpty {
				halt(reboot: self.pendingStopAction == .reboot, update: self.pendingStopAction == .update)
			}
		}
	}
	
	fileprivate func handleCommand(_ message: ChatMessage) {
		var components = message.content.lowercased().components(separatedBy: CharacterSet.whitespaces)
		components.removeFirst()
		
		var args = [String]()
		
		for command in commands {
			let usages = command.usage()
			
			for i in 0..<usages.count {
				let usage = usages[i]
				
				var match = true
				let usageComponents = usage.components(separatedBy: CharacterSet.whitespaces)
				let lastIndex = min(components.count, usageComponents.count)
				
				for i in 0..<lastIndex {
					let component = components[i]
					let usageComponent = usageComponents[i]
					
					if usageComponent == "*" {
						args.append(component)
					}
					else if usageComponent == "..." {
						//everything else is arguments; add them to the list
						args.append(contentsOf: components[i..<components.count])
					}
					else if component != usageComponent {
						match = false
					}
				}
				
				
				let minCount = usageComponents.last! == "..." ? lastIndex - 1 : lastIndex
				if components.count < minCount {
					match = false
				}
				
				
				if match {
					runCommand(command.init(bot: self, message: message, arguments: args, usageIndex: i))
					return
				}
			}
		}
	}
	
	func chatRoomMessage(_ room: ChatRoom, message: ChatMessage, isEdit: Bool) {
		let lowercase = message.content.lowercased()
		if pendingStopAction == .run && lowercase.hasPrefix("@fir") {
			//do a more precise check so names like @FirstStep won't cause the bot to respond
			let name = "@firealarm".unicodeScalars
			
			let msg = lowercase.unicodeScalars
			
			for i in 0...name.count {
				if i >= msg.count {
					if i > 4 {
						break
					}
					else {
						return
					}
				}
				let messageChar = msg[msg.index(msg.startIndex, offsetBy: i)]
				if i < name.count {
					let nameChar = name[name.index(name.startIndex, offsetBy: i)]
					if !CharacterSet.alphanumerics.contains(nameChar) {
						break
					}
					if nameChar != messageChar {
						return
					}
				}
				else {
					if CharacterSet.alphanumerics.contains(messageChar) {
						return
					}
				}
				
			}
			handleCommand(message)
		}
	}
	
	func stop(_ stopAction: StopAction) {
		pendingStopAction = stopAction
	}
	
	init(_ room: ChatRoom) {
		self.room = room
		filter = Filter(self)
	}
}
