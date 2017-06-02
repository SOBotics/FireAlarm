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
        guard
            let url = URL(string: arguments[0]),
            let questionID = postIDFromURL(url),
            let siteDomain = url.host
            else {
                reply("Please enter a valid post URL.")
                return
        }
        
        if reporter == nil {
            reply("Waiting for the filter to load...")
            repeat {
                sleep(1)
            } while reporter == nil
        }
        
        guard
            let site = try reporter.staticDB.run(
                "SELECT id FROM sites WHERE domain = ?",
                siteDomain
                ).first?.column(at: 0) as Int? else {
                    
                    reply("That does not look like a site on which I run.")
                    return
        }
        
        guard
            let question = try apiClient.fetchQuestion(
                questionID,
                parameters: ["site":siteDomain]
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
