//
//  CommandGetBlacklistedKeywords.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/15/17.
//
//

import Foundation
import SwiftChatSE

class CommandGetBlacklistedKeywords: Command {
    override class func usage() -> [String] {
        return ["blacklisted keywords"]
    }
    
    override func run() throws {
        let filter = reporter.filter(ofType: FilterBlacklistedKeywords.self)
        
        if filter?.blacklistedKeywords.count == 0 {
            reply("No keywords are blacklisted.")
            return
        }
        reply("Blacklisted keywords:")
        post("    " + (filter?.blacklistedKeywords.joined(separator: "\n    ") ?? ""))
    }
}
