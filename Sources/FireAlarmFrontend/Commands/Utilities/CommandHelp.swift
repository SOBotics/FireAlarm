//
//  CommandHelp.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandHelp: Command {
    override class func usage() -> [String] {
        return ["help", "command", "commands"]
    }
    
    override func run() throws {
        reply("I'm  [\(botName)](\(stackAppsLink)), a bot which detects questions that need closing. [My command list is available here](https://github.com/SOBotics/FireAlarm/wiki/CommandsSwift).")
    }
}
