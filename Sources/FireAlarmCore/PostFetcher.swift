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
    open var postsToCheck = [(id: Int, site: String)]()
    
    public let queue = DispatchQueue(label: "Filter", attributes: [.concurrent])
    
    
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
    
    open func doCheckPosts() {
        queue.async {
            do {
                while true {
                    let posts = self.postsToCheck
                    sleep(60)
                    if !self.running {
                        return
                    }
                    
                    guard !posts.isEmpty else {
                        continue
                    }
                    
                    //print("Checking \(posts.count) posts.")
                    self.postsToCheck = self.postsToCheck.filter { post in
                        !posts.contains { $0.id == post.id && $0.site == post.site }
                    }
                    
                    var postsBySite = [String:[Int]]()
                    for post in posts {
                        if postsBySite[post.site] != nil {
                            postsBySite[post.site]!.append(post.id)
                        } else {
                            postsBySite[post.site] = [post.id]
                        }
                    }
                    
                    for (site, posts) in postsBySite {
                        for post in try self.apiClient.fetchQuestions(
                            posts,
                            parameters: ["site":site]
                            ).items ?? [] {
                                
                                //don't report posts that are more than a day old
                                let creation = (post.creation_date ?? Date()).timeIntervalSinceReferenceDate
                                let activity = (post.last_activity_date ?? Date()).timeIntervalSinceReferenceDate
                                
                                if creation < (activity - 60 * 60 * 24) {
                                    continue
                                }
                                
                                try self.callback(post, site)
                                
                                guard self.shouldFetchAnswers else { continue }
                                for answer in post.answers ?? [] {
                                    answer.title = post.title
                                    answer.tags = post.tags
                                    
                                    //don't report answers that are more than a day old
                                    let creation = (answer.creation_date ?? Date()).timeIntervalSinceReferenceDate
                                    let activity = (answer.last_activity_date ?? Date()).timeIntervalSinceReferenceDate
                                    
                                    if creation < (activity - 60 * 60 * 24) {
                                        continue
                                    }
                                    
                                    try self.callback(answer, site)
                                }
                        }
                    }
                }
            } catch {
                handleError(error, "while fetching posts")
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
                
                postsToCheck.append((id: id, site: apiSiteParameter))
                
                /*let trollSite: Site?
                 switch reporter.trollSites {
                 case .all:
                 guard let domain = data["siteBaseHostAddress"] as? String else {
                 throw QuestionProcessingError.noSiteBaseHostAddress(json: string)
                 }
                 trollSite = Site(id: -1, apiSiteParameter: apiSiteParameter, domain: domain, initialProbability: 0)
                 case .sites(let sites): trollSite = sites.filter { $0.apiSiteParameter == apiSiteParameter }.first
                 }
                 
                 if let site = trollSite {
                 postsToCheck.append((id: id, site: site))
                 } else {
                 guard let site = try Site.with(apiSiteParameter: apiSiteParameter, db: staticDB) else {
                 return
                 }
                 
                 postsToCheck.append((id: id, site: site))
                 }*/
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
