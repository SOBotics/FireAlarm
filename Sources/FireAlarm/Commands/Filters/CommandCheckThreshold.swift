//
//  CommandCheckThreshold.swift
//  FireAlarm
//
//  Created by NobodyNada on 2/9/17.
//
//

import Foundation
import SwiftChatSE

class CommandCheckThreshold: Command {
    override class func usage() -> [String] {
        return ["threshold", "check threshold", "get threshold", "current threshold",
                "thresholds","check thresholds","get thresholds","current thresholds"
        ]
    }
    
    override func run() throws {
        if message.room.thresholds.isEmpty {
            reply("This room does not report any posts.")
        } else if message.room.thresholds.count == 1 {
            let threshold = message.room.thresholds.first!.value
            reply(
                "The threshold for this room is \(threshold) (*higher* thresholds report more posts)."
            )
        } else {
            let siteNames: [String] = try message.room.thresholds.keys.map {
                try Site.with(id: $0, db: reporter.staticDB)?.domain ?? "<unknown site \($0)>"
            }
            let siteThresholds = Array(message.room.thresholds.values.map(String.init))
            
            reply("The thresholds for this room are: (*higher* thresholds report more posts)")
            post(makeTable(["Site", "Threshold"], contents: siteNames, siteThresholds))
        }
    }
}
