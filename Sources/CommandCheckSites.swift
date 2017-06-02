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
            try reporter.staticDB.run(
                "SELECT domain FROM sites WHERE id = ?", $0
                ).first?.column(at: 0) ?? "<unknown site \($0)>"
        }
        
        reply("This room reports posts from \(formatArray(siteNames, conjunction: "and")).")
    }
}
