//
//  CommandRemoveSite.swift
//  FireAlarm
//
//  Created by NobodyNada on 6/1/17.
//
//

import Foundation
import SwiftChatSE

class CommandRemoveSite: Command {
    override class func usage() -> [String] {
        return ["remove site *"]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .owner
    }
    
    override func run() throws {
        guard
            let siteName = arguments.first,
            let site: Int = try reporter.staticDB.run(
                "SELECT id FROM sites WHERE domain = ? OR apiSiteParameter = ?",
                siteName, siteName
                ).first?.column(at: 0),
            message.room.thresholds[site] != nil else {
                
                reply("Please enter a site and a threshold.")
                return
        }
        
        message.room.thresholds[site] = nil
        
        reply("Removed \(siteName) from this room's sites.")
    }
}
