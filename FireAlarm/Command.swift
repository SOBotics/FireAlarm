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
    
    let message: ChatMessage    //The message that triggered this command.
    let bot: ChatBot
    var finished = false    //Whether the command has completed execution.  Will be set to true automatically.
    let arguments: [String]
    
    func run() throws {
        fatalError("run() must be overridden")
    }
    
    required init(bot: ChatBot, message: ChatMessage, arguments: [String]) {
        self.bot = bot
        self.message = message
        self.arguments = arguments
    }
}
