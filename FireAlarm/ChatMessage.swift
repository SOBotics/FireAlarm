//
//  ChatMessage.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

open class ChatMessage {
    open let user: ChatUser
    open let content: String
    open let id: Int?
    
    public init(user: ChatUser, content: String, id: Int?) {
        self.user = user
        self.content = content
        self.id = id
    }
}
