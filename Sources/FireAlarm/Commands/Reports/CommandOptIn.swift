//
//  CommandOptIn.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandOptIn: Command {
    override class func usage() -> [String] {
        return ["notify ...", "opt in ...", "opt-in ..."]
    }
    
    func addNotificationTags() {
        if arguments.count < 2 {
            reply("Please specify which tags.")
            return
        }
        
        let existingTags = message.user.notificationReasons.flatMap {
            if case .tag(let t) = $0 { return t }
            return nil
        }
        
        for tag in arguments.dropFirst() {
            if !existingTags.contains(tag) {
                message.user.notificationReasons.append(.tag(tag))
            }
        }
        
        let newTags = message.user.notificationReasons.flatMap {
            if case .tag(let t) = $0 { return t }
            return nil
        }
        
        let formatted = formatArray(newTags.map { "[tag:\($0)]" }, conjunction: "or")
        reply("You will be notified of reports tagged \(formatted).")
    }
    
    func addNotificationBlacklist() {
        if arguments.count < 2 {
            reply("Please specify which blacklists; recognized blacklists are `keywords`, `tags`, and `usernames`..")
            return
        }
        
        let existingBlacklists = message.user.notificationReasons.flatMap {
            if case .blacklist(let l) = $0 { return l.rawValue }
            return nil
        }
        
        var reasonsToAdd = [ChatUser.NotificationReason]()
        for blacklist in arguments.dropFirst() {
            if !existingBlacklists.contains(blacklist) {
                guard let list = BlacklistManager.BlacklistType(name: blacklist) else {
                    reply("Unrecognized blacklist \(blacklist); recognized blacklists are `keywords`, `tags`, and `usernames`.")
                    return
                }
                reasonsToAdd.append(.blacklist(list))
            }
        }
        message.user.notificationReasons += reasonsToAdd
        
        let newBlacklists = message.user.notificationReasons.flatMap {
            if case .blacklist(let l) = $0 { return l.rawValue + "s" }
            return nil
        }
        
        let formatted = formatArray(newBlacklists, conjunction: "or")
        reply("You will be notified of posts containing blacklisted \(formatted).")
    }
    
    func addNotificationLinks() {
        let containsMisleadingLinks = message.user.notificationReasons.contains {
            if case .misleadingLinks = $0 { return true }
            else { return false }
        }
        if containsMisleadingLinks {
            reply("You're already notified of misleading link reports.")
        } else {
            message.user.notificationReasons.append(.misleadingLinks)
            reply("You will now be notified of misleading link reports.")
        }
    }
    
    override func run() throws {
        
        if arguments.count == 0 {
            message.user.notificationReasons = [.all]
            reply("You will now be notified of all reports.")
        }
        else {
            if ["misleading links", "misleading link", "misleadinglink", "misleadinglinks", "link", "links"]
                .contains(arguments.joined(separator: " ")) {
                addNotificationLinks()
            } else {
                switch arguments.first! {
                case "tag", "tags":
                    addNotificationTags()
                case "blacklist", "blacklists", "blacklisted":
                    addNotificationBlacklist()
                default:
                    reply("Unrecognized reason \"\(arguments.first!)\"; allowed reasons are `tags`, `misleading links`, and `blacklists`.")
                }
            }
        }
        
    }
}
