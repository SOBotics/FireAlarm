//
//  CommandWhy.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 06/05/17.
//
//

import Foundation
import SwiftChatSE

class CommandWhy: Command {
	override class func usage() -> [String] {
		return ["why"]
	}
	
	override func run() throws {
		let matchingReport: Report
		
		if let replyID = message.replyID {
			
			//This is a reply.
			//Get the last report whose message matches this one.
			if let report = reportedPosts
				.reversed()
				.first(where: { report in
					report.messages.contains { $0.messageID == replyID && $0.host == message.room.host}
				}) {
				
				matchingReport = report
			} else {
				reply("That message is not in the report list.")
				return
			}
		} else {
			//This is not a reply.
			//Get the last report in this room.
			if let report = reportedPosts
				.reversed()
				.first(where: { report in
					report.messages.contains { $0.roomID == message.room.roomID }
				}) {
				
				matchingReport = report
			} else {
				reply("No reports have been posted in this room.")
				return
			}
		}
        
        if let details = matchingReport.details {
            reply("Detected with likelihood \(matchingReport.likelihood): \(details)")
        } else {
            reply("Details for this post are not available (cc @AshishAhuja @NobodyNada).")
        }
	}
}
