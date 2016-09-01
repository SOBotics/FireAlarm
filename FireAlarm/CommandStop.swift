//
//  CommandStop.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 8/31/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandStop: Command {
    private let REBOOT_INDEX = 4
    override class func usage() -> [String] {
        return ["stop", "halt", "shutdown", "shut down", "restart", "reboot"]
    }
    
    override func run() throws {
        let action: ChatBot.StopAction
        let reply: String
        if usageIndex < REBOOT_INDEX {
            action = .Halt
            reply = "Shutting down..."
        }
        else {
            action = .Reboot
            reply = "Rebooting..."
        }
        bot.room.postReply(reply, to: message)
        bot.stop(action)
    }
}