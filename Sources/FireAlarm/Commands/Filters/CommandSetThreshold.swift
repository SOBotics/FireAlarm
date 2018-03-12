//
//  CommandSetThreshold.swift
//  FireAlarm
//
//  Created by NobodyNada on 2/9/17.
//
//

import Foundation
import SwiftChatSE

class CommandSetThreshold: Command {
    override public class func usage() -> [String] {
        return [
            "set threshold to * for *",
            "set threshold to *",
            "change threshold to * for *",
            "change threshold to *",
            "set threshold ...",
            "change threshold ..."
        ]
    }
    
    override public class func privileges() -> ChatUser.Privileges {
        return .filter
    }
    
    override func run() throws {
        guard let newThreshold = (arguments.first.map({Int($0)}) ?? nil) else {
            message.reply("Please specify a valid threshold.")
            return
        }
        
        if arguments.count < 2 {
            if message.room.thresholds.count == 1 {
                message.room.thresholds[message.room.thresholds.first!.key] = newThreshold
            } else {
                message.reply("Which site would you like to change threshold for?")
            }
        } else {
            guard let site = try reporter.staticDB.run("SELECT * FROM sites " +
                "WHERE apiSiteParameter = ? OR domain = ?",
                                      arguments.last!, arguments.last!
                ).first.map(Site.from) else {
                    
                    message.reply("That does not look like a site on which I run.")
                    return
            }
            guard message.room.thresholds[site.id] != nil else {
                message.reply("I do not report posts from that site here.")
                return
            }
            
            message.room.thresholds[site.id] = newThreshold
            reply("Set threshold for `\(site.domain)` to \(newThreshold).")
        }
    }
}
