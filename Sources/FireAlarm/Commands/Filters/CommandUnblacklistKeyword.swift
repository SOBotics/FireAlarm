//
//  CommandUnblacklistKeyword.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/15/17.
//
//

import Foundation
import SwiftChatSE

class CommandUnblacklistKeyword: Command {
    override class func usage() -> [String] {
        return ["unblacklist keyword ...", "unblacklist ..."]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .filter
    }
    
    override func run() throws {
        if arguments.count == 0 {
            reply("Please specify the keyword to remove from the blacklist.")
            return
        }
        
        let keyword = arguments.joined(separator: " ")
        let filter = reporter.filter(ofType: FilterBlacklistedKeywords.self)
        
        if let index = filter?.blacklistedKeywords.index(of: keyword) {
            filter?.blacklistedKeywords.remove(at: index)
            reply("\(keyword) was removed from the blacklist.")
        } else {
            reply("\(keyword) was not blacklisted.")
        }
    }
}
