//
//  CommandReport.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 2/26/17.
//  Copyright © 2017 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

import Foundation
import SwiftChatSE
import SwiftStack
import Dispatch

class CommandReport: Command {
    override class func usage() -> [String] {
        return ["report ..."]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .owner
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
        
        
        
        var filterResult = try reporter.checkPost(question, site: site)
        
        filterResult.append(FilterResult (type: .manuallyReported, header: "Manually reported question", details: "Question manually reported by \(message.user): https://\(message.room.host.chatDomain)/transcript/message/\(message.id ?? -1)#\(message.id ?? -1)"))
        
        switch reporter.report(post: question, reasons: filterResult) {
        case .alreadyClosed:
            reply("That post is already closed.")
        case .alreadyReported:
            reply("That post has already been reported.")
        case .notBad:
            reply("this should never happen (cc @NobodyNada): `\(#file)`, line \(#line)")
        case .reported(_):
            break
        }
    }
}
