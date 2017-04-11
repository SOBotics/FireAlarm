//
//  CommandPingOnError.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 09/04/17.
//
//

import Foundation
import SwiftChatSE

class CommandPingOnError: Command {
    override class func usage() -> [String] {
        return ["pingonerror ..."]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .owner
    }
    
    override func run() throws {
        if (arguments.count == 0) {
            message.reply ("Usage: `@fire pingonerror <true|false>`")
            return
        }
        
        if (arguments [0] == "true") {
            pingonerror = true
            message.reply ("The owner of this instance will now be pinged during errors.")
        } else if (arguments [0] == "false") {
            pingonerror = false
            message.reply ("The owner of this instance will now not be pinged during errors.")
        } else {
            message.reply ("Usage: `@fire pingonerror <true|false>`")
        }
    }
}
