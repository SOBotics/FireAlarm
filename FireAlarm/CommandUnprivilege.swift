//
//  CommandUnprivilege.swift
//  FireAlarm
//
//  Created by NobodyNada on 11/21/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class CommandUnprivilege: Command {
	override class func usage() -> [String] {
		return ["unprivilege * *", "remove privilege * *"]
	}
	
	override class func privileges() -> ChatUser.Privileges {
		return .owner
	}
	
	private func usage() {
		if usageIndex == 0 {
			reply("Usage: unprivilege <user> <privilege>")
		} else {
			reply("Usage: remove privilege <user> <privilege>")
		}
	}
	
	override func run() throws {
		if arguments.count != 2 {
			return usage()
		}
		
		let user = arguments.first!
		let privilegeName = arguments.last!
		
		var privilege: ChatUser.Privileges?
		
		for (priv, name) in ChatUser.Privileges.privilegeNames {
			if name.lowercased() == privilegeName.lowercased()  {
				privilege = ChatUser.Privileges(rawValue: priv)
				break
			}
		}
		
		guard let priv = privilege else {
			reply("\(privilegeName) is not a valid privilege")
			return
		}
		
		var targetUser: ChatUser?
		let idFromURL: Int?
		if let url = URL(string: user), let id = postIDFromURL(url, isUser: true) {
			idFromURL = id
		} else {
			idFromURL = nil
		}
		
		//search for the user in the user database
		for chatUser in message.room.userDB {
			if chatUser.id == Int(user) ||
				chatUser.name.replacingOccurrences(of: " ", with: "").lowercased() == user.lowercased() ||
				chatUser.id == idFromURL {
				
				targetUser = chatUser
				break
			}
		}
		
		guard let u = targetUser else {
			reply("I don't know that user.")
			return
		}
		
		guard u.privileges.contains(priv) else {
			reply("That user doesn't have that privilege.")
			return
		}
		
		u.privileges.subtract(priv)
		reply("Removed \(ChatUser.Privileges.name(of: priv)) privileges from [\(u.name)](//stackoverflow.com/u/\(u.id)).")
	}
}
