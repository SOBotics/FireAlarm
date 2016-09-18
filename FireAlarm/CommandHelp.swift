//
//  CommandHelp.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Cocoa

class CommandHelp: Command {
    override class func usage() -> [String] {
        return ["help"]
    }
    
    override func run() throws {
        bot.room.postReply("I'm  [FireAlarm](https://github.com/NobodyNada/chatbot), a bot which detects questions that need closing. [My command list is available here](https://github.com/NobodyNada/chatbot/wiki/Commands).", to: message)
        bot.room.postMessage("(Note: Most of those commands have not yet been implemented in this Swift rewrite.)")
    }
}
