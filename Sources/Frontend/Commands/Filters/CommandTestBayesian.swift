//
//  CommandTestBayesian.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 27/03/17.
//  Copyright © 2017 Ashish Ahuja. All right reserved
//

import Foundation
import SwiftChatSE
import SwiftStack

class CommandTestBayesian: Command {
    override class func usage() -> [String] {
        return ["test ..."]
    }
    
    override func run () throws {
        /*guard let body = arguments.map({"\($0)"}).joined(separator: " ") ?? nil else {
            message.reply ("Please enter a valid body")
            return
        }*/
        
        //Under development ...
    }
}
