//
//  ChatMessage.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class ChatMessage {
    let user: ChatUser
    let content: String
    let id: Int?
    
    init(user: ChatUser, content: String, id: Int?) {
        self.user = user
        self.content = content
        self.id = id
    }
}
