//
//  ChatBot.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Cocoa

class ChatBot: ChatRoomDelegate {
    let room: ChatRoom
    
    let commands: [Command.Type] = [
        CommandTest.self,
        CommandHelp.self, CommandListRunning.self, CommandStop.self
    ]
    
    let commandQueue = DispatchQueue(label: "Command Queue", attributes: DispatchQueue.Attributes.concurrent)
    
    var runningCommands = [Command]()
    
    enum StopAction {
        case run
        case halt
        case reboot
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
            if (self.pendingStopAction == .halt || self.pendingStopAction == .reboot) && self.runningCommands.isEmpty {
                halt(reboot: self.pendingStopAction == .reboot)
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
        if pendingStopAction == .run && message.content.lowercased().hasPrefix("@fir") {
            handleCommand(message)
        }
    }
    
    func stop(_ stopAction: StopAction) {
        pendingStopAction = stopAction
    }
    
    init(_ room: ChatRoom) {
        self.room = room
    }
}
