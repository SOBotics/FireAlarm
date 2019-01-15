//
//  CommandUnblacklistKeyword.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/15/17.
//
//

import Foundation
import SwiftChatSE

class CommandUnblacklist: Command {
    public class func blacklistManager(reporter: Reporter) -> BlacklistManager {
        return reporter.blacklistManager
    }
    
    override class func usage() -> [String] {
        return ["unblacklist ..."]
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
        if type(of: self).blacklistManager(reporter: reporter).blacklist(ofType: list).remove(item: regex) == false {
            reply("`\(regex)` was not blacklisted.")
        } else {
            reply("`\(regex)` has been removed from the \(list.rawValue) blacklist.")
        }
    }
}
