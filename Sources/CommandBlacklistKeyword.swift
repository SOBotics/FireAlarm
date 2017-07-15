//
//  CommandBlacklistKeyword.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/15/17.
//
//

import Foundation
import SwiftChatSE

class CommandBlacklistKeyword: Command {
    override class func usage() -> [String] {
        return ["blacklist keyword ...", "blacklist ..."]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .filter
    }
    
    
    override func run() throws {
        if arguments.count == 0 {
            reply("Please sepcify the keyword to blacklist.")
            return
        }
        let regex = arguments.joined(separator: " ")
        reporter.filter(ofType: FilterBlacklistedKeywords.self)?.blacklistedKeywords.append(regex)
        reply("Blacklisted regular expression `\(regex)`.")
    }
}
