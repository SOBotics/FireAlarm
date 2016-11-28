//
//  CommandStop.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/31/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandStop: Command {
    fileprivate let REBOOT_INDEX = 4
    override class func usage() -> [String] {
        return ["stop", "halt", "shutdown", "shut down", "restart", "reboot"]
    }
	
	override class func privileges() -> ChatUser.Privileges {
		return [.owner]
	}
    
    override func run() throws {
        let action: ChatListener.StopAction
        let reply: String
        if usageIndex < REBOOT_INDEX {
            action = .halt
            reply = "Shutting down..."
        }
        else {
            action = .reboot
            reply = "Rebooting..."
        }
        self.reply(reply)
        listener.stop(action)
    }
}
