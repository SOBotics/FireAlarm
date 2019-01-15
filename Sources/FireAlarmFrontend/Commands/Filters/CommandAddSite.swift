//
//  CommandAddSite.swift
//  FireAlarm
//
//  Created by NobodyNada on 6/1/17.
//
//

import Foundation
import SwiftChatSE

class CommandAddSite: Command {
    override class func usage() -> [String] {
        return ["add site * *", "add site * threshold *"]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .owner
    }
    
    override func run() throws {
        guard arguments.count == 2 else {
            reply("Please enter a site and a threshold.")
            return
        }
        
        let siteName = arguments[0]
        guard let threshold = Int(arguments[1]) else {
            reply("Please enter a valid threshold.")
            return
        }
        
        guard let site = try reporter.staticDB.run(
            "SELECT * FROM sites WHERE domain = ? OR apiSiteParameter = ?",
            siteName, siteName
            ).first.map(Site.from) else {
                
                reply("That does not look like a site on which I run.")
                return
        }
        
        message.room.thresholds[site.id] = threshold
        
        reply("Added \(site.domain) to this room's sites with threshold \(threshold).")
    }
}
