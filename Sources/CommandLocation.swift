//
//  CommandLocation.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 25/02/17.
//  Copyright © 2017 Ashish Ahuja. All right reserved
//

import Foundation
import SwiftChatSE

class CommandLocation: Command {
    override class func usage() -> [String] {
        return ["location"]
    }
    
    override func run() throws {
        //Get the location
        let message = "I'm running on \(location)"
        reply (message)
    }
}

