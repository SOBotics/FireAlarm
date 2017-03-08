//
//  CommandUnclosed.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 05/03/17.
//  Copyright © 2017 Ashish Ahuja. All right reserved
//

import Foundation
import SwiftStack
import SwiftChatSE
import Dispatch

class CommandUnclosed: Command {
    override class func usage() -> [String] {
        return ["unclosed reports ..."]
    }
    
    override func run() throws {
        
        if (reportedPosts.isEmpty)
        {
            reply ("no reports!")
            return
        }
        
        var totalToCheck: Int!
        
        if let total = Int(arguments[0]) {
            if (total > 20)
            {
                reply ("You cannot specify to check more than the last 20 reports!")
                return
            }
            totalToCheck = total
        } else {
            reply ("Please enter how many reports should be checked!")
            return
        }
        
        var postsToCheck = [Int]()
        
        if let minDate: Date = Calendar(identifier: .gregorian).date(byAdding: DateComponents(hour: -10), to: Date()) {
            let recentlyReportedPosts = reportedPosts.filter {
                $0.when > minDate
            }
            
            if (recentlyReportedPosts.count == 0)
            {
                reply ("There are no reports made by me recently, thus I have nothing to check!")
                return
            }
            
            if (totalToCheck > recentlyReportedPosts.count)
            {
                totalToCheck = recentlyReportedPosts.count
            }
            
            for i in 0..<totalToCheck {
                postsToCheck.append(recentlyReportedPosts[i].id)
            }
        }
        else {
            message.room.postMessage("Failed to calculate minimum report date!")
        }
        
        var messageClosed = ""
        
        //Now fetch the posts from the API
        for post in try apiClient.fetchQuestions(postsToCheck).items ?? [] {
            if (post.closed_reason == nil)
            {
                //post is not closed
                //message.room.postMessage("\(post.link)")
                
                messageClosed = messageClosed + "\n [tag: \(post.tags!.first)] \(post.link!)"
            }
        }
        
        message.room.postMessage (messageClosed)
    }
}
