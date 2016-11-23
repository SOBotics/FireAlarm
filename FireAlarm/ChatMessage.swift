//
//  ChatMessage.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

open class ChatMessage {
	open let room: ChatRoom
    open let user: ChatUser
    open let content: String
    open let id: Int?
	open let replyID: Int?
	
	public init(room: ChatRoom, user: ChatUser, content: String, id: Int?, replyID: Int? = nil) {
		self.room = room
        self.user = user
        self.content = content
        self.id = id
		self.replyID = replyID
    }
}
