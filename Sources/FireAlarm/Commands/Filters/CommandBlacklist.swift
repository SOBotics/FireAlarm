//
//  CommandBlacklist.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/15/17.
//
//

import Foundation
import SwiftChatSE

class CommandBlacklist: Command {
    public class func blacklistManager(reporter: Reporter) -> BlacklistManager {
        return reporter.blacklistManager
    }
    
    override class func usage() -> [String] {
        return ["blacklist ...", "blacklist ..."]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .filter
    }
    
    
    override func run() throws {
        guard arguments.count >= 2,
            let listName = arguments.first,
            let list = BlacklistManager.BlacklistType(rawValue: listName) else {
                reply("Usage: `blacklist <keyword|username|tag> <item>`")
                return
        }
        let regex = arguments.dropFirst().joined(separator: " ")
        if type(of: self).blacklistManager(reporter: reporter).blacklist(ofType: list).add(item: regex) == false {
            reply("`\(regex) was already on the \(list.rawValue) blacklist.")
        } else {
            reply("Added regular expression `\(regex)` to the \(list.rawValue) blacklist.")
        }
    }
}
