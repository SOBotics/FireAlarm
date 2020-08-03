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

open class PostFetcher {
    open var postsToCheck = [String:[Int]]() // Posts by apiSiteParameter
    
    public let queue = DispatchQueue(label: "Filter")
    
    
    private var ws: WebSocket!
    private var wsRetries: Int
    private let wsMaxRetries: Int
    
    public var apiClient: APIClient
    
    /// A callback that recieves a post and its apiSiteParameter.
    open var callback: (Post, String) throws -> ()
    
    open var shouldFetchAnswers = false
    
    public private(set) var running: Bool
    
    private var lastEventDate: Date?
    
    public init (apiClient: APIClient, callback: @escaping (Post, String) throws -> ()) {
        wsRetries = 0
        wsMaxRetries = 10
        running = false
        
        self.apiClient = apiClient
        self.callback = callback
    }
    
    public enum QuestionProcessingError: Error {
        case textNotUTF8(text: String)
        
        case jsonNotDictionary(json: String)
        case jsonParsingError(json: String, error: String)
        case noDataObject(json: String)
        case noQuestionID(json: String)
        case noSite(json: String)
        case noSiteBaseHostAddress(json: String)
        
        case siteLookupFailed(siteID: Int)
    }
    
    open func checkStackOverflow() {
        Thread.detachNewThread {
            var lastCheck = Date()
            while true {
                do {
                    let wakeTime = lastCheck.addingTimeInterval(60)
                    lastCheck = Date()
                    Thread.sleep(until: wakeTime)
                    
                    if !self.running {
                        return
                    }
                    
                    var hasMore = true
                    var page = 1
                    var posts: [Post] = []
                    while hasMore {
                        let response = try self.queue.sync { try self.apiClient.fetchQuestions(parameters: [
                            "pagesize": "100",
                            "page": "\(page)",
                            "fromdate": String(Int(lastCheck.timeIntervalSince1970)),
                            "site": "stackoverflow"
                        ])}
                        hasMore = response.has_more ?? false
                        page += 1
                        posts.append(contentsOf: response.items ?? [])
                    }
                    
                    let questionIDs = posts.compactMap { $0.id }
                    
                    // now fetch answers
                    hasMore = true
                    page = 1
                    while hasMore {
                        let response = try self.queue.sync { try self.apiClient.fetchAnswersOn(
                            questions: questionIDs,
                            parameters: [
                                "pagesize": "100",
                                "page": "\(page)",
                                "site": "stackoverflow"
                            ])}
                        hasMore = response.has_more ?? false
                        page += 1
                        posts.append(contentsOf: response.items ?? [])
                    }
                    
                    self.queue.sync {
                        posts.forEach { post in
                            do {
                                //don't report posts that are more than a day old
                                let creation = (post.creation_date ?? Date()).timeIntervalSinceReferenceDate
                                let activity = (post.last_activity_date ?? Date()).timeIntervalSinceReferenceDate
                                
                                if creation > (activity - 60 * 60 * 24) {
                                    try self.callback(post, "stackoverflow")
                                }
                            } catch {
                                handleError(error, "while processing a post")
                            }
                        }
                    }
                } catch {
                    handleError(error, "while fetching questions on Stack Overflow")
                }
            }
        }
    }
    
    open func doCheckPosts() {
        Thread.detachNewThread {
            while true {
                do {
                    // Wait 60 seconds due to API caching.
                    let posts = self.postsToCheck
                    sleep(60)
                    if !self.running {
                        return
                    }
                    
                    guard !posts.isEmpty else {
                        continue
                    }
                    
                    // Remove the posts we're checking now from the list of postsToCheck.
                    for site in self.postsToCheck.keys {
                        let postsToCheckNow = posts[site] ?? []
                        self.postsToCheck[site] = self.postsToCheck[site]?.filter { post in
                            !postsToCheckNow.contains(post)
                        }
                    }
                    
                    for (site, posts) in posts {
                        if posts.isEmpty { continue }
                        var fetchedPosts = [Post]()
                        
                        var hasMore = true
                        var page = 1
                        while hasMore {
                            let response = try self.queue.sync { try self.apiClient.fetchQuestions(
                                posts, parameters: [
                                    "pagesize": "100",
                                    "page": "\(page)",
                                    "site": site
                                ])}
                            hasMore = response.has_more ?? false
                            page += 1
                            fetchedPosts.append(contentsOf: response.items ?? [])
                        }
                        
                        let questionIDs = fetchedPosts.compactMap { $0.id }
                        
                        // now fetch answers
                        hasMore = true
                        page = 1
                        while hasMore {
                            let response = try self.queue.sync { try self.apiClient.fetchAnswersOn(
                                questions: questionIDs,
                                parameters: [
                                    "pagesize": "100",
                                    "page": "\(page)",
                                    "site": site
                                ])}
                            hasMore = response.has_more ?? false
                            page += 1
                            fetchedPosts.append(contentsOf: response.items ?? [])
                        }
                        
                        self.queue.sync {
                            fetchedPosts.forEach { post in
                                do {
                                    //don't report posts that are more than a day old
                                    let creation = (post.creation_date ?? Date()).timeIntervalSinceReferenceDate
                                    let activity = (post.last_activity_date ?? Date()).timeIntervalSinceReferenceDate
                                    
                                    if creation > (activity - 60 * 60 * 24) {
                                        try self.callback(post, site)
                                    }
                                } catch {
                                    handleError(error, "while processing a post")
                                }
                            }
                        }
                    }
                } catch {
                    handleError(error, "while fetching posts")
                }
            }
        }
    }
    
    open func start() throws {
        running = true
        
        //let request = URLRequest(url: URL(string: "ws://qa.sockets.stackexchange.com/")!)
        //ws = WebSocket(request: request)
        //ws.eventQueue = room.client.queue
        //ws.delegate = self
        //ws.open()
        ws = try WebSocket.open("wss://qa.sockets.stackexchange.com/")
        
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
        
        doCheckPosts()
    }
    
    open func stop() {
        running = false
        ws?.disconnect()
    }
    
    private func webSocketOpen() {
        print("Listening to active questions!")
        wsRetries = 0
        ws.write("155-questions-active")
        
        Thread.detachNewThread {
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
    
    private func webSocketClose(_ code: Int, reason: String, wasClean: Bool) {
        //do nothing -- we'll handle this in webSocketEnd
    }
    
    private func webSocketError(_ error: NSError) {
        //do nothing -- we'll handle this in webSocketEnd
    }
    
    private func webSocketMessageText(_ text: String) {
        do {
            guard let data = text.data(using: .utf8) else {
                throw QuestionProcessingError.textNotUTF8(text: text)
            }
            webSocketMessageData(data)
        } catch {
            handleError(error, "while processing an active question")
        }
    }
    
    private func webSocketMessageData(_ data: Data) {
        lastEventDate = Date()
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
                
                guard let id = data["id"] as? Int else {
                    throw QuestionProcessingError.noQuestionID(json: string)
                }
                
                guard let apiSiteParameter = data["apiSiteParameter"] as? String else {
                    throw QuestionProcessingError.noSite(json: string)
                }
                
                if apiSiteParameter != "stackoverflow" {    // SO questions are handled seperately
                    postsToCheck[apiSiteParameter, default: []].append(id)
                }
                
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
                try ws.connect()
                done = true
            } catch {
                done = false
                sleep(5)
            }
        } while !done
    }
    
    private func webSocketEnd(_ code: Int, reason: String, wasClean: Bool, error: Error?) {
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
