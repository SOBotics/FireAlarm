//
//  CommandCheckPost.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandCheckPost: Command {
	override class func usage() -> [String] {
		return ["check post *", "check *"]
	}
	
	override func run() throws {
		var questionID: Int!
		if let id = Int(arguments[0]) {
			questionID = id
		}
		else if let url = URL(string: arguments[0]), let id = postIDFromURL(url) {
			questionID = id
		}
		else {
			reply("Please enter a valid post ID or URL.")
			return
		}
		
		let result = try listener.filter.checkAndReportPost(message.room.client.questionWithID(questionID))
		switch result {
		case .notBad:
			reply("That post was not caught by the filter.")
		case .alreadyReported:
			reply("That post was already reported.")
		case .reported:
			break
		}
	}
}
