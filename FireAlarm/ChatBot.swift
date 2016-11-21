//
//  ChatBot.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import Dispatch

open class ChatBot: ChatRoomDelegate {
	open let room: ChatRoom
	
	open let commands: [Command.Type]
	
	open var info: Any? = nil
	
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
		let required = type(of: command).privileges()
		let missing = command.message.user.missing(from: required)
		
		
		guard missing.isEmpty else {
			let message = "You need the \(formatArray(missing.names, conjunction: "and")) " +
			"\(pluralize(missing.names.count, "privilege")) to run that command."
			
			room.postReply(message, to: command.message)
			return
		}
		
		
		
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
		
		var commandScores = [String:Int]()
		
		for command in commands {
			let usages = command.usage()
			
			for i in 0..<usages.count {
				var score = 0
				let usage = usages[i]
				args = []
				
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
				
				
				let minCount = usageComponents.last! == "..." ? usageComponents.count - 1 : usageComponents.count
				if components.count < minCount {
					match = false
				}
				
				
				if match {
					runCommand(command.init(bot: self, message: message, arguments: args, usageIndex: i))
					return
				}
				else {
					//Determine how similar the input was to this command.
					//Higher score means more differences.
					var availableComponents = components	//The components which have not been "used."
					//Each component may only be matched once.
					var availableUsageComponents = usageComponents.filter {
						$0 != "*" && $0 != "..."
					}
					
					//While there are unused components, iterate over all available components and remove the closest pairs.
					
					while !availableComponents.isEmpty && !availableUsageComponents.isEmpty {
						var bestMatch: (score: Int, component: String, usageComponent: String)?
						
						for usageComponent in availableUsageComponents {
							if usageComponent == "*" || usageComponent == "..." {
								continue
							}
							
							for component in availableComponents {
								let distance = Levenshtein.distanceBetween(usageComponent, and: component)
								let componentScore = min(distance, usageComponent.characters.count)
								
								if componentScore < bestMatch?.score ?? Int.max {
									bestMatch = (score: componentScore, component: component, usageComponent: usageComponent)
								}
							}
						}
						
						
						if let (compScore, comp, usageComp) = bestMatch {
							score += compScore
							availableComponents.remove(at: availableComponents.index(of: comp)!)
							availableUsageComponents.remove(at: availableUsageComponents.index(of: usageComp)!)
						}
					}
					
					
					let args = usageComponents.filter {
						$0 == "*" || $0 == "..."
					}
					for _ in args {
						if !availableComponents.isEmpty {
							availableComponents.removeFirst()
						}
					}
					
					for component in (availableComponents + availableUsageComponents) {
						score += component.characters.count
					}
					
					commandScores[usage] = score
				}
			}
		}
		
		
		
		
		var lowest: (command: String, score: Int)?
		for (command, score) in commandScores {
			if score <= command.characters.count/2 && score < (lowest?.score ?? Int.max) {
				lowest = (command, score)
			}
		}
		
		if let (command, _) = lowest {
			room.postReply("Unrecognized command `\(components.joined(separator: " "))`; did you mean `\(command)`?", to: message)
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
	
	func postIDFromURL(_ url: URL, isUser: Bool = false) -> Int? {
		if url.host != "stackoverflow.com" && url.host != "www.stackoverflow.com" {
			return nil
		}
		
		let componentIndex: Int
		let component: String
		if url.pathComponents.first == "/" {
			if url.pathComponents.count < 3 {
				return nil
			}
			componentIndex = 1
		}
		else {
			if url.pathComponents.count < 2 {
				return nil
			}
			componentIndex = 0
		}
		component = url.pathComponents[componentIndex]
		
		if (isUser && (component == "u" || component == "users")) ||
			(!isUser && (component == "q" || component == "questions" ||
				component == "a" || component == "answers" ||
				component == "p" || component == "posts")) {
			return Int(url.pathComponents[componentIndex + 1])
		}
		return nil
	}
	
	func stop(_ stopAction: StopAction) {
		pendingStopAction = stopAction
		filter.stop()
		if self.runningCommands.isEmpty {
			halt(reboot: self.pendingStopAction == .reboot, update: self.pendingStopAction == .update)
		}
	}
	
	init(_ room: ChatRoom, commands: [Command.Type]) {
		self.room = room
		self.commands = commands
		filter = Filter(self)
	}
}
