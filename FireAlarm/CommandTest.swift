//
//  CommandTest.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandTest: Command {
    override class func usage() -> [String] {
        return ["test"]
    }
    
    override func run() throws {
        reply("Test")
    }
}
