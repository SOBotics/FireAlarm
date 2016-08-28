//
//  ChatRoom.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/27/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class ChatRoom: NSObject, WebSocketDelegate {
    enum ChatEvent: Int {
        case MessagePosted = 1
        case MessageEdited = 2
        case UserEntered = 3
        case UserLeft = 4
        case RoomNameChanged = 5
        case MessageStarred = 6
        case DebugMessage = 7
        case UserMentioned = 8
        case MessageFlagged = 9
        case MessageDeleted = 10
        case FileAdded = 11
        case ModeratorFlag = 12
        case UserSettingsChanged = 13
        case GlobalNotification = 14
        case AccessLevelChanged = 15
        case UserNotification = 16
        case Invitation = 17
        case MessageReply = 18
        case MessageMovedOut = 19
        case MessageMovedIn = 20
        case TimeBreak = 21
        case FeedTicker = 22
        case UserSuspended = 29
        case UserMerged = 30
        case UsernameChanged = 34
    };
    
    
    
    let client: Client
    let roomID: Int
    
    var ws: WebSocket!
    private var wsRetries = 0
    private let wsMaxRetries = 10
    
    var inRoom = false
    
    var timestamp: Int = 0
    
    private var messageQueue = [String]()
    
    init(client: Client, roomID: Int) {
        self.client = client
        self.roomID = roomID
    }
    
    enum RoomJoinError: ErrorType {
        case RoomInfoRetrievalFailed
    }
    
    private func connectWS() throws {
        //get the timestamp
        guard let time = (try client.parseJSON(client.post("https://chat.\(client.host.rawValue)/chats/\(roomID)/events", [
            "roomid" : roomID,
            "fkey": client.fkey
            ])) as? NSDictionary)?["time"] as? Int else {
                throw RoomJoinError.RoomInfoRetrievalFailed
        }
        timestamp = time
        
        //get the auth code
        let wsAuth = try client.parseJSON(
            client.post("https://chat.\(client.host.rawValue)/ws-auth", ["roomid":roomID, "fkey":client.fkey]
            )
            ) as! NSDictionary
        
        let wsURL = wsAuth["url"] as! String
        
        let request = NSMutableURLRequest(URL: NSURL(string: "\(wsURL)?l=\(timestamp)")!)
        
        request.setValue("https://chat.\(client.host.rawValue)", forHTTPHeaderField: "Origin")
        
        ws = WebSocket(request: request)
        ws.eventQueue = client.queue
        ws.delegate = self
    }
    
    private func messageQueueHandler() {
        while messageQueue.count != 0 {
            var result: String? = nil
            do {
                result = try client.post(
                    "https://chat.\(client.host.rawValue)/chats/\(roomID)/messages/new",
                    ["text":messageQueue[0], "fkey":client.fkey]
                )
            }
            catch {
                handleError(error)
            }
            do {
                if let json = result {
                    let _ = try client.parseJSON(json)
                    messageQueue.removeFirst()
                }
            }
            catch {
                print(result)
            }
            sleep(1)
        }
    }
    
    func postMessage(message: String) {
        messageQueue.append(message)
        if messageQueue.count == 1 {
            dispatch_async(client.queue) {
                self.messageQueueHandler()
            }
        }
    }
    
    func join() throws {
        print("Joining chat room \(roomID)...")
        
        try connectWS()
    }
    
    enum EventError: ErrorType {
        case JSONParsingFailed
        case InvalidEventType
    }
    
    func handleEvents(events: NSArray) throws {
        for e in events {
            guard let event = e as? NSDictionary else {
                throw EventError.JSONParsingFailed
            }
            guard let typeCode = event["event_type"] as? Int else {
                throw EventError.JSONParsingFailed
            }
            guard let type = ChatEvent(rawValue: typeCode) else {
                throw EventError.InvalidEventType
            }
            
            switch type {
            case .MessagePosted, .MessageEdited:
                guard
                    let user = event["user_name"] as? String,
                    let content = event["content"] as? String else {
                        throw EventError.JSONParsingFailed
                }
                
                print("\(user): \(content)")
            default:
                break
            }
        }
    }
    
    func webSocketOpen() {
        print("Websocket opened!")
        wsRetries = 0
    }
    
    func webSocketClose(code: Int, reason: String, wasClean: Bool) {
        //do nothing -- we'll handle this in webSocketEnd
    }
    
    func webSocketError(error: NSError) {
        //do nothing -- we'll handle this in webSocketEnd
    }
    
    @objc func webSocketMessageText(text: String) {
        do {
            guard let json = try client.parseJSON(text) as? NSDictionary else {
                throw EventError.JSONParsingFailed
            }
            
            let roomKey = "r\(roomID)"
            guard let events = (json[roomKey] as? NSDictionary)?["e"] as? NSArray else {
                throw EventError.JSONParsingFailed
            }
            
            try handleEvents(events)
        }
        catch {
            handleError(error)
        }
    }
    
    @objc func webSocketMessageData(data: NSData) {
        print("Recieved binary data: \(data)")
    }
    
    func webSocketEnd(code: Int, reason: String, wasClean: Bool, error: NSError?) {
        if let e = error {
            print("Websocket error:\n\(e)")
        }
        else {
            print("Websocket closed")
        }
        
        if inRoom {
            if wsRetries >= wsMaxRetries {
                fatalError("Failed to reconnect websocket!")
            }
            print("Trying to reconnect...")
            webSocketOpen()
        }
    }
    
}
