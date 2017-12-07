//
//  CommandGetBlacklist.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/15/17.
//
//

import Foundation
import SwiftChatSE

class CommandGetBlacklist: Command {
    override class func usage() -> [String] {
        return ["blacklisted *"]
    }
    
    override func run() throws {
        guard let listName = arguments.first,
            let listType = BlacklistManager.BlacklistType(name: listName) else {
                    reply("Usage: `blacklisted <keywords|usernames|tags>`")
                    return
        }
        
        let list = reporter.blacklistManager.blacklist(ofType: listType)
        if list.items.count == 0 {
            reply("No \(listType.rawValue)s are blacklisted.")
            return
        }
        reply("Blacklisted \(listType.rawValue)s:")
        post("    " + (list.items.joined(separator: "\n    ")))
    }
}
