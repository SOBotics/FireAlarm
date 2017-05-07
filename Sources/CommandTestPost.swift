//
//  CommandTestPost.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 05/05/17.
//
//

/*import Foundation
import SwiftChatSE

class CommandTestPost: Command {
    override class func usage() -> [String] {
        return ["test post *", "test *"]
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
        
        guard let question = try apiClient.fetchQuestion(questionID).items?.first else {
            reply("Could not fetch the question!")
            return
        }
        
        let result = reporter.checkPost(post: question)
        
        switch result {
            case 
        }
    }
}*/
