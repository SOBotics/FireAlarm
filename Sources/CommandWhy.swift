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
        var index = reportedPosts.count - 1
        print ("\(index)")
        
        while (index >= 0) {
            if reportedPosts[index].messageID == message.replyID {
                message.reply(reportedPosts [index].details)
                return
            }
            index -= 1
        }
        
        message.reply ("Not a report (or did the bot reboot after the report?) \(index)")
    }
}
