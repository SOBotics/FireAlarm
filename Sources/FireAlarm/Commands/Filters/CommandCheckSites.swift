//
//  CommandCheckSites.swift
//  FireAlarm
//
//  Created by NobodyNada on 6/1/17.
//
//

import Foundation
import SwiftChatSE

class CommandCheckSites: Command {
    override class func usage() -> [String] {
        return ["sites", "check sites", "get sites", "current sites"]
    }
    
    override func run() throws {
        if message.room.thresholds.isEmpty {
            reply("This room does not report any posts.")
            
            return
        }
        
        let siteNames: [String] = try message.room.thresholds.keys.map {
            try Site.with(id: $0, db: reporter.staticDB)?.domain ?? "<unknown site \($0)>"
        }
        
        reply("This room reports posts from \(formatArray(siteNames, conjunction: "and")).")
    }
}
