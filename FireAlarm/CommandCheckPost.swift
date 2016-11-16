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
		else if let url = URL(string: arguments[0]), let id = bot.postIDFromURL(url) {
			questionID = id
		}
		else {
			bot.room.postReply("Please enter a valid post ID or URL.", to: message)
			return
		}
		
		let result = try bot.filter.checkAndReportPost(bot.room.client.questionWithID(questionID))
		switch result {
		case .notBad:
			bot.room.postReply("That post was not caught by the filter.", to: message)
		case .alreadyReported:
			bot.room.postReply("That post was already reported.", to: message)
		case .reported:
			break
		}
	}
}
