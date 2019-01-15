//
//  CommandUpdate.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandUpdate: Command {
    fileprivate let FORCE_INDEX = 2
    override class func usage() -> [String] {
        return ["update force ...", "pull force ...", "update ...", "pull ..."]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .owner
    }
    
    override func run() throws {
        if noUpdate {
            reply("Updates are disabled for this instance.")
            return
        }
        
        let commit: String?
        if arguments.count > 1 {
            reply("Usage: update [force] [revision]")
            return
        }
        else if arguments.count == 1 {
            commit = arguments.first!
        } else {
            commit = nil
        }
        
        if isUpdating {
            reply("An update is already in progress.")
            return
        }
        if (!update (to: commit, listener: listener, rooms: [message.room], force: (usageIndex < FORCE_INDEX)))
        {
            reply ("No new update available.")
        }
        return
    }
}
