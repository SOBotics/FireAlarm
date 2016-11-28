//
//  Command.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

open class Command {
    ///Returns an array of possible usages.  * means a parameter; ... means a variable number of parameters.
    open class func usage() -> [String] {
        fatalError("usage() must be overriden")
    }
	
	open class func privileges() -> ChatUser.Privileges {
		return []
	}
    
    ///The message that triggered this command.
    open let message: ChatMessage
    open let listener: ChatListener
    
    ///Whether the command has completed execution.  Will be set to true automatically by ChatListener.
    open internal(set) var finished = false
    
    open let arguments: [String]
    
    ///Which usage of the command was run.  Useful for implementing
    ///commands that share most of their code, like shutdown/reboot.
    open let usageIndex: Int
	
	
	///Replies to the message.
	open func reply(_ reply: String) {
		message.room.postReply(reply, to: message)
	}
	
	open func post(_ message: String) {
		self.message.room.postMessage(message)
	}
		
    
    open func run() throws {
        fatalError("run() must be overridden")
    }
    
    public required init(listener: ChatListener, message: ChatMessage, arguments: [String], usageIndex: Int = 0) {
        self.listener = listener
        self.message = message
        self.arguments = arguments
        self.usageIndex = usageIndex
    }
}
