//
//  CommandCheckNotification.swift
//  FireAlarm
//
//  Created by NobodyNada on 10/1/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandCheckNotification: Command {
	override class func usage() -> [String] {
		return ["notified", "aminotified", "am i notified", "will i be notified", "opted-in", "opted in", "am i opted in", "have i opted in"]
	}
	
	override func run() throws {
		if message.user.notificationReasons.isEmpty {
			reply("You will not be notified of any reports.")
		}
        else if message.user.notificationReasons.contains(where: { if case .all = $0 { return true } else { return false } }) {
			reply("You will be notified of all reports.")
		}
		else {
			var reasons: [String] = []
            
            let tags = message.user.notificationReasons.compactMap { (reason: ChatUser.NotificationReason) -> String? in
                if case .tag(let tag) = reason { return tag }
                else { return nil }
            }
            
            let blacklists = message.user.notificationReasons.compactMap {
                if case .blacklist(let list) = $0 { return list }
                else { return nil }
            } as [BlacklistManager.BlacklistType]
            
            let containsMisleadingLink = message.user.notificationReasons.contains {
                if case .misleadingLinks = $0 { return true }
                else { return false }
            }
            
            
			if !tags.isEmpty {
				let formattedTags = formatArray(tags.map { "[tag:\($0)]" }, conjunction: "or")
				reasons.append("reports tagged \(formattedTags)")
			}
			if !blacklists.isEmpty {
                let formattedBlacklists = formatArray(blacklists.map { "\($0.rawValue)s" }, conjunction: "and")
				reasons.append("blacklisted \(formattedBlacklists)")
			}
            if containsMisleadingLink {
                reasons.append("misleading links")
            }
			let formatted = formatArray(reasons, conjunction: "and")
			reply("You will be notified of \(formatted).")
		}

	}
}
