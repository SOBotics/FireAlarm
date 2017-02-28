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
        return ["report print ...", "report ..."]
    }
    
    override class func privileges() -> ChatUser.Privileges {
        return .owner
    }
    
    func tags(for post: Post) -> [String] {
        if let q = post as? Question {
            return q.tags ?? []
        } else if let a = post as? Answer {
            return a.tags ?? []
        } else {
            return []
        }
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
        
        guard let question = try apiClient.fetchQuestion(questionID).items?.first else {
            reply("Could not fetch the question!")
            return
        }
        
        if usageIndex == 0
        {
            let messagePost = "[ [\(botName)](\(stackAppsLink)) ] " +
            "[tag:\(tags(for: question).first ?? "<unknown tag>")] Manually reported post [\(question.title ?? "<no title>")](//stackoverflow.com/q/\(questionID))"
            message.room.postMessage(messagePost)
        }
    }
}
