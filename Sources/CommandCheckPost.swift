//
//  CommandCheckPost.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

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
		
		if reporter == nil {
			reply("Waiting for the filter to load...")
			repeat {
				sleep(1)
			} while reporter == nil
		}
        
        let apiSiteParameter = "stackoverflow"
        guard let site = try reporter.staticDB.run(
            "SELECT * FROM sites WHERE apiSiteParameter = ?",
            apiSiteParameter
            ).first?.column(at: 0) as Int? else {
                
                reply("Could not fetch the site!")
                return
        }
        
		
        guard let question = try apiClient.fetchQuestion(
            questionID,
            parameters: ["site":apiSiteParameter]
            ).items?.first else {
                
                reply("Could not fetch the question!")
                return
        }
        
        let result = try reporter.checkAndReportPost(question, site: site)
		switch result {
            case .alreadyClosed:
                reply ("That post was caught by the filter but is already closed.")
            case .notBad:
                reply("That post was not caught by the filter.")
            case .alreadyReported:
                reply("That post was already reported.")
            case .reported:
                break
		}
	}
}
