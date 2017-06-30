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
    
    let githubSecret: String
    
    init(githubSecret: String) {
        self.githubSecret = githubSecret
    }
    
    
    func process(event: Redunda.Event, rooms: [ChatRoom]) throws {
        guard event.name == "ci_status" else { return }
        
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
            let repoName = content["name"] else {
                throw GithubWebhookError.invalidPayload(payload: event.content)
        }
        
        let targetURL = content["target_url"] as? String
        
        if state == "success" {
            //TODO: autoupdate
        }
        
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
    }
}
