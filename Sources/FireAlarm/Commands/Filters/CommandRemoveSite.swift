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
            let site = try reporter.staticDB.run(
                "SELECT * FROM sites WHERE domain = ? OR apiSiteParameter = ?",
                siteName, siteName
                ).first.map(Site.from),
            message.room.thresholds[site.id] != nil else {
                
                reply("Please enter a site and a threshold.")
                return
        }
        
        message.room.thresholds[site.id] = nil
        
        reply("Removed \(site.domain) from this room's sites.")
    }
}
