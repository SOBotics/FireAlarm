//
//  Command.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Cocoa

class Command {
    ///Returns an array of possible usages.  * means a parameter; ... means a variable number of parameters.
    class func usage() -> [String] {
        fatalError("usage() must be overriden")
    }
    
    ///The message that triggered this command.
    let message: ChatMessage
    let bot: ChatBot
    
    ///Whether the command has completed execution.  Will be set to true automatically by ChatBot.
    var finished = false
    
    let arguments: [String]
    
    ///Which usage of the command was run.  Useful for implementing
    ///commands that share most of their code, like shutdown/reboot.
    let usageIndex: Int
    
    func run() throws {
        fatalError("run() must be overridden")
    }
    
    required init(bot: ChatBot, message: ChatMessage, arguments: [String], usageIndex: Int = 0) {
        self.bot = bot
        self.message = message
        self.arguments = arguments
        self.usageIndex = usageIndex
    }
}
