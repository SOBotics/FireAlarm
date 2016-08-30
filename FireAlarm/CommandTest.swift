//
//  CommandTest.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Cocoa

class CommandTest: Command {
    override class func usage() -> [String] {
        return ["test"]
    }
    
    override func run() throws {
        bot.room.postReply("Test", to: message)
        bot.room.postMessage(makeTable(["Test1", "Test2", "Test3"], contents:
            ["abc","def","ghi"],
            ["0", "123456"],
            ["The quick brown fox jumps over the lazy dog.", "test"]
            ))
    }
}
