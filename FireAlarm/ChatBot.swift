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
    
    let commandQueue = dispatch_queue_create("Command Queue", DISPATCH_QUEUE_CONCURRENT)
    
    var runningCommands = [Command]()
    
    enum StopAction {
        case Run
        case Halt
        case Reboot
    }
    
    private var pendingStopAction = StopAction.Run
    
    private func runCommand(command: Command) {
        runningCommands.append(command)
        dispatch_async(commandQueue) {
            do {
                try command.run()
            }
            catch {
                handleError(error, "while running \"\(command.message.content)\"")
            }
            self.runningCommands.removeAtIndex(self.runningCommands.indexOf {$0 === command}!)
            if (self.pendingStopAction == .Halt || self.pendingStopAction == .Reboot) && self.runningCommands.isEmpty {
                halt(reboot: self.pendingStopAction == .Reboot)
            }
        }
    }
    
    private func handleCommand(message: ChatMessage) {
        var components = message.content.lowercaseString.componentsSeparatedByCharactersInSet(NSCharacterSet.whitespaceCharacterSet())
        components.removeFirst()
        
        var args = [String]()
        
        for command in commands {
            let usages = command.usage()
            
            for i in 0..<usages.count {
                let usage = usages[i]
                
                var match = true
                let usageComponents = usage.componentsSeparatedByCharactersInSet(NSCharacterSet.whitespaceCharacterSet())
                let lastIndex = min(components.count, usageComponents.count)
                
                for i in 0..<lastIndex {
                    let component = components[i]
                    let usageComponent = usageComponents[i]
                    
                    if usageComponent == "*" {
                        args.append(component)
                    }
                    else if usageComponent == "..." {
                        //everything else is arguments; add them to the list
                        args.appendContentsOf(components[i..<components.count])
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
    
    func chatRoomMessage(room: ChatRoom, message: ChatMessage, isEdit: Bool) {
        if pendingStopAction == .Run && message.content.lowercaseString.hasPrefix("@fir") {
            handleCommand(message)
        }
    }
    
    func stop(stopAction: StopAction) {
        pendingStopAction = stopAction
    }
    
    init(_ room: ChatRoom) {
        self.room = room
    }
}
