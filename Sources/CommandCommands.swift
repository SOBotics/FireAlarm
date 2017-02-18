//
//  CommandCommands.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 2/18/2017.
//  Copyright © 2016 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandCommands: Command {
    override class func usage() -> [String] {
        return ["command", "commands"]
    }
    
    override func run() throws {
        reply("[My command list is available here](https://github.com/SOBotics/FireAlarm/wiki/CommandsSwift).")
    }
}
