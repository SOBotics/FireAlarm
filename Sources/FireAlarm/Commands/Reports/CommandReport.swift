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
        
        
        
        var filterResult = try reporter.checkPost(question, site: site)
        
        filterResult.append(FilterResult (type: .manuallyReported, header: "Manually reported question", details: "Question manually reported by \(message.user): https://\(message.room.host.chatDomain)/transcript/message/\(message.id ?? -1)#\(message.id ?? -1)"))
        
        switch try reporter.report(post: question, site: site, reasons: filterResult).status {
        case .alreadyClosed:
            reply("That post is already closed.")
        case .alreadyReported:
            reply("That post has already been reported.")
        case .notBad:
            reply("this should never happen (cc @NobodyNada): `\(#file)`, line \(#line)")
        case .reported:
            break
        }
    }
}
