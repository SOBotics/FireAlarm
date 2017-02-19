//
//  CommandLeaveRoom.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 02/19/17.
//  Copyright © 2017 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

import Foundation
import SwiftChatSE

class CommandLeaveRoom: Command {
	override class func usage() -> [String] {
		return ["leave room", "leave"]
	}
	
	override class func privileges() -> ChatUser.Privileges {
		return .owner
	}
	
	override func run() throws {
    [message.room].forEach { $0.leave() }
	}
}
