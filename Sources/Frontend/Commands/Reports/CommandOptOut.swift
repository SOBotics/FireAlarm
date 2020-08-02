//
//  CommandOptOut.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandOptOut: Command {
    override class func usage() -> [String] {
        return ["opt out ...", "opt-out ...", "unnotify"]
    }
    
    func removeNotificationTags() {
        message.user.notificationReasons = message.user.notificationReasons.filter {
            if case .tag(let tag) = $0 {
                return !arguments.contains(tag)
            }
            return true
        }
        
        let formatted = formatArray(arguments.dropFirst().map { "[tag:\($0)]" }, conjunction: "or")
        reply("You will not be notified of posts tagged \(formatted).")
    }
    
    func removeNotificationBlacklists() {
        message.user.notificationReasons = message.user.notificationReasons.filter {
            if case .blacklist(let list) = $0 {
                return !(arguments.contains(list.rawValue) || arguments.contains(list.rawValue + "s"))
            }
            return true
        }
        
        let formatted = formatArray(Array(arguments.dropFirst()), conjunction: "or")
        reply("You will not be notified of blacklisted \(formatted) reports.")
    }
    
    func removeNotificationLinks() {
        guard let index = message.user.notificationReasons.firstIndex(where: {
            if case .misleadingLinks = $0 { return true }
            else { return false }
        }) else {
            reply("You are not currently notified of misleading link reports.")
            return
        }
        message.user.notificationReasons.remove(at: index)
        reply("You will not be notified of misleading link reports.")
    }
    
    override func run() throws {
        if arguments.isEmpty || message.user.notificationReasons.isEmpty {
            message.user.notificationReasons = []
            reply("You will not be notified of any reports.")
        }
        else {
            switch arguments.first! {
            case "tag", "tags":
                removeNotificationTags()
            case "blacklisted", "blacklist", "blacklists":
                removeNotificationBlacklists()
            case "misleading", "misleadinglink", "misleadinglinks", "link", "links":
                removeNotificationLinks()
            default:
                reply("Unrecognized reason \"\(arguments.first!)\"; allowed reasons are `tags`, `blacklists`, and `misleading links`.")
            }
        }
    }
}
