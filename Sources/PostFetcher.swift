//
//  PostFetcher.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 24/04/17.
//
//

import Foundation
import SwiftChatSE
import SwiftStack
import Dispatch

class PostFetcher {
    var postsToCheck = [Int]()
    
    var queue = DispatchQueue(label: "Filter", attributes: [.concurrent])
    
    var ws: WebSocket!
    fileprivate var wsRetries: Int
    fileprivate let wsMaxRetries: Int
    
    let rooms: [ChatRoom]
    weak var classifier: PostClassifier!
    
    private(set) var running: Bool
    
    private var lastEventDate: Date?
    
	init (rooms: [ChatRoom], classifier: PostClassifier) {
        wsRetries = 0
        wsMaxRetries = 10
        running = false
        
        self.rooms = rooms
        
        self.classifier = classifier
    }
    
    enum QuestionProcessingError: Error {
        case textNotUTF8(text: String)
        
        case jsonNotDictionary(json: String)
        case jsonParsingError(json: String, error: String)
        case noDataObject(json: String)
        case noQuestionID(json: String)
        case noSite(json: String)
    }
    
    func doCheckPosts() {
        queue.async {
            while true {
                do {
                    let posts = self.postsToCheck
                    sleep(60)
                    if !self.running {
                        return
                    }
                    
                    guard !posts.isEmpty else {
                        continue
                    }
                    
                    //print("Checking \(posts.count) posts.")
                    self.postsToCheck = self.postsToCheck.filter {!posts.contains($0)}
                    for post in try apiClient.fetchQuestions(posts).items ?? [] {
                        //don't report posts that are more than a day old
                        let creation = (post.creation_date ?? Date()).timeIntervalSinceReferenceDate
                        let activity = (post.last_activity_date ?? Date()).timeIntervalSinceReferenceDate
                        
                        if creation < (activity - 60 * 60 * 24) {
                            continue
                        }
                        
                        try self.classifier.checkAndReportPost(post)
                    }
                } catch {
                    handleError(error, "while checking active posts")
                }
            }
        }
    }

    func start() throws {
        running = true
        
        //let request = URLRequest(url: URL(string: "ws://qa.sockets.stackexchange.com/")!)
        //ws = WebSocket(request: request)
        //ws.eventQueue = room.client.queue
        //ws.delegate = self
        //ws.open()
        ws = try WebSocket("wss://qa.sockets.stackexchange.com/")
        
        ws.onOpen {socket in
            self.webSocketOpen()
        }
        ws.onText {socket, text in
            self.webSocketMessageText(text)
        }
        ws.onBinary {socket, data in
            self.webSocketMessageData(data)
        }
        ws.onClose {socket in
            self.webSocketClose(0, reason: "", wasClean: true)
            self.webSocketEnd(0, reason: "", wasClean: true, error: socket.error)
        }
        ws.onError {socket in
            self.webSocketEnd(0, reason: "", wasClean: true, error: socket.error)
        }
        
        try ws.connect()
        
        doCheckPosts()
    }
    
    func stop() {
        running = false
        ws?.disconnect()
    }
    
    func webSocketOpen() {
        print("Listening to active questions!")
        wsRetries = 0
        ws.write("155-questions-active")
        
        queue.async {
            while true {
                sleep(600)
                if (Date().timeIntervalSinceReferenceDate -
                    (self.lastEventDate?.timeIntervalSinceReferenceDate ?? 0)) > 600 {
                    
                    self.ws?.disconnect()
                }
                return
            }
        }
    }
    
    func webSocketClose(_ code: Int, reason: String, wasClean: Bool) {
        //do nothing -- we'll handle this in webSocketEnd
    }
    
    func webSocketError(_ error: NSError) {
        //do nothing -- we'll handle this in webSocketEnd
    }
    
    func webSocketMessageText(_ text: String) {
        do {
            guard let data = text.data(using: .utf8) else {
                throw QuestionProcessingError.textNotUTF8(text: text)
            }
            webSocketMessageData(data)
        } catch {
            handleError(error, "while processing an active question")
        }
    }
    
    func webSocketMessageData(_ data: Data) {
        let string = String(data: data, encoding: .utf8) ?? "<not UTF-8: \(data.base64EncodedString())>"
        do {
            
            do {
                guard let json = try JSONSerialization.jsonObject(with: data, options: []) as? [String:String] else {
                    throw QuestionProcessingError.jsonNotDictionary(json: string)
                }
                
                guard json["action"] == "155-questions-active" else {
                    if json["action"] == "hb" {
                        //heartbeat
                        ws.write("{\"action\":\"hb\",\"data\":\"hb\"}")
                    }
                    return
                }
                
                guard let dataObject = json["data"]?.data(using: .utf8) else {
                    throw QuestionProcessingError.noDataObject(json: string)
                }
                
                guard let data = try JSONSerialization.jsonObject(with: dataObject, options: []) as? [String:Any] else {
                    throw QuestionProcessingError.noDataObject(json: string)
                }
                
                guard let site = data["apiSiteParameter"] as? String else {
                    throw QuestionProcessingError.noSite(json: string)
                }
                
                guard site == "stackoverflow" else {
                    return
                }
                
                guard let id = data["id"] as? Int else {
                    throw QuestionProcessingError.noQuestionID(json: string)
                }
                
                
                postsToCheck.append(id)
                //print("Another post has been recieved.  There are now \(postsToCheck.count) posts to check.")
                
            } catch {
                if let e = errorAsNSError(error) {
                    throw QuestionProcessingError.jsonParsingError(json: string, error: formatNSError(e))
                } else {
                    throw QuestionProcessingError.jsonParsingError(json: string, error: String(describing: error))
                }
            }
        }
        catch {
            handleError(error, "while processing an active question")
        }
    }

    
    private func attemptReconnect() {
        var done = false
        repeat {
            do {
                if wsRetries >= wsMaxRetries {
                    fatalError(
                        "Realtime questions websocket died; failed to reconnect!  Active posts will not be reported until a reboot. \(ping)"
                    )
                }
                wsRetries += 1
                try start()
                done = true
            } catch {
                done = false
                sleep(5)
            }
        } while !done
    }
    
    func webSocketEnd(_ code: Int, reason: String, wasClean: Bool, error: Error?) {
        if let e = error {
            print("Websocket error:\n\(e)")
        }
        else {
            print("Websocket closed")
        }
        
        if running {
            print("Trying to reconnect...")
            attemptReconnect()
        }
    }
}
