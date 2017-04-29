//
//  CommandUpdate.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandUpdate: Command {
    fileprivate let FORCE_INDEX = 2
	override class func usage() -> [String] {
		return ["update force ...", "pull force ...", "update ...", "pull ..."]
	}
	
	override class func privileges() -> ChatUser.Privileges {
		return .owner
	}
	
    override func run() throws {
		if noUpdate {
			reply("Updates are disabled.")
			return
		}
		
        let argLocation: String
        
        if (arguments.count == 0)
        {
            if (!update (listener, [message.room], force: (usageIndex < FORCE_INDEX), auto: false))
            {
                reply ("No new update available.")
            }
            return
        } else {
            argLocation = arguments.joined(separator: " ").lowercased()
        }
        
        if (userLocation == "<unknown>") {
            reply("The current instance's location is unknown.")
            return
        }
        
        if (userLocation.lowercased() == argLocation)
        {
            if (!update (listener, [message.room], force: (usageIndex < FORCE_INDEX), auto: false))
            {
                reply ("No new update available.")
            }
            return
        }
	}
}
