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
        
        if (arguments.count == 0) {
            reply ("Please specify the number of posts to be checked.")
            return
        }
        
        var totalToCheck: Int!
        let maxThreshold = 100
        var threshold: Int! = maxThreshold
        
        if let total = Int(arguments[0]) {
            if (total > 40)
            {
                reply ("You cannot specify to check more than the last 40 reports!")
                return
            }
            totalToCheck = total
        } else {
            reply ("Please enter a valid number for the total reports to be checked.")
            return
        }
        
        if (arguments.count > 1) {
            for i in 1..<arguments.count {
                if (arguments[i].range(of: "t=") != nil || arguments[i].range(of: "threshold=") != nil) {
                    if (arguments[i].range(of: "t=") != nil) {
                        threshold = Int (arguments[i].replacingOccurrences(of: "t=", with: ""))
                    } else if arguments[i].range(of: "threshold=") != nil {
                        threshold = Int (arguments[i].replacingOccurrences(of: "threshold=", with: ""))
                    }
                    
                    if (threshold > maxThreshold)
                    {
                        threshold = maxThreshold
                    }
                }
            }
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
            
            var i = 0
            while postsToCheck.count < totalToCheck {
                if (recentlyReportedPosts [i].difference < threshold) {
                    postsToCheck.append(recentlyReportedPosts[i].id)
                }
                
                i = i + 1
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
				messageClosed = messageClosed + "\nCV:\(post.close_vote_count ?? 0)   \(post.link?.absoluteString ?? "https://example.com")"
            }
        }
        
        message.room.postMessage (messageClosed)
    }
}
