//
//  WebhookHandler.swift
//  FireAlarm
//
//  Created by NobodyNada on 6/8/17.
//
//

import CryptoSwift
import SwiftChatSE

class WebhookHandler {
    enum GithubWebhookError: Error {
        case invalidSignature
        case invalidPayload(payload: String)
    }
    enum UpdateToWebhookError: Error {
        case invalidToken
        case invalidPayload
    }
    
    let githubSecret: String
    
    init(githubSecret: String) {
        self.githubSecret = githubSecret
    }
    
    //A closure to be called when CI succeeds.  The closure is passed the repo name, commit branhces, and commit SHA.
    var successHandler: ((String, [String], String) -> ())?
    
    //A closure to be called when an update_to event is recievd.  The closure is passed the commit SHA.
    var updateHandler: ((String) -> ())?
    
    func onSuccess(_ handler: ((String, [String], String) -> ())?) { successHandler = handler }
    func onUpdate(_ handler: ((String) -> ())?) { updateHandler = handler }
    
    
    func process(event: Redunda.Event, rooms: [ChatRoom]) throws {
        let eventHandlers = [
            "ci_status":processCIStatus,
            "update_to":processUpdateTo
        ]
        
        try eventHandlers[event.name]?(event, rooms)
    }
    
    
    
    private func processCIStatus(event: Redunda.Event, rooms: [ChatRoom]) throws {
        let hmac = try HMAC(key: githubSecret, variant: .sha1)
        let signature = try hmac.authenticate(event.content.data(using: .utf8)!.bytes).toHexString()
        guard "sha1=" + signature == event.headers["X-Hub-Signature"] else {
            print("Invalid event signature (expected \(signature)) for:\n\(event)")
            throw GithubWebhookError.invalidSignature
        }
        
        print("Recieved GitHub webhook event.")
        guard event.headers["X-Github-Event"] == "status" else { return }
        
        guard let content = try event.contentAsJSON() as? [String:Any],
            let commitHash = content["sha"] as? String,
            let state = content["state"] as? String,
            let repoName = content["name"] as? String,
            let branches = (content["branches"] as? [[String:Any]])?.flatMap({ $0["name"] as? String })
            
            else {
                throw GithubWebhookError.invalidPayload(payload: event.content)
        }
        
        let targetURL = content["target_url"] as? String
        
        if state == "pending" {
            return
        }
        
        let repoLink = "https://github.com/\(repoName)"
        
        let header = "[ [\(repoName)](\(repoLink)) ]"
        let link = targetURL != nil ? "[CI](\(targetURL!))" : "CI"
        let status = [
            "pending": "pending",
            "success": "succeeded",
            "failure": "failed",
            "error": "errored"
            ][state] ?? "status unknown"
        let commitLink = "[\(getShortVersion(commitHash))](\(repoLink)/commit/\(commitHash))"
        
        let message = [header, link, status, "on", commitLink].joined(separator: " ") + "."
        rooms.forEach { $0.postMessage(message) }
        
        if state == "success" {
            successHandler?(repoName, branches, commitHash)
        }
    }
    
    private func processUpdateTo(event: Redunda.Event, rooms: [ChatRoom]) throws {
        guard let json = try event.contentAsJSON() as? [String:Any],
            let commit = json["commit"] as? String else {
                throw UpdateToWebhookError.invalidPayload
        }
        guard json["token"] as? String == githubSecret else {
            throw UpdateToWebhookError.invalidToken
        }
        
        updateHandler?(commit)
        
    }
}
