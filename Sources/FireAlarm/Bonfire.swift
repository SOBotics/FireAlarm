//
//  Bonfire.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 14/04/18.
//

import Foundation
import Dispatch
import SwiftChatSE
import SwiftStack

class Bonfire {
    enum BonfireError: Error {
        case postCreationFailed(status: Int)
        case invalidJSON(json: Any)
    }
    
    let key: String
    let client: Client
    let host: String
    
    func getPostLink(_ bonfire_post_id: Int) -> String {
        return self.host + "/posts/\(bonfire_post_id)"
    }
    
    func uploadPost(post: Post, postDetails: String?, likelihood: Int) throws -> String {
        //TODO: Looks like SwiftStack does not have a 'creation_date' in the Post class; add it. Currently just using last activity date.
        let dateFormatter = DateFormatter()
        dateFormatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss.SSSZ"
        let creation_date = dateFormatter.string(from: post.last_activity_date!)
        
        var reasonList = [String]()
        let reasons = postDetails!.components(separatedBy: ", ")
        for reason in reasons {
            if reason.range(of: "manually reported") != nil {
                reasonList.append("Manually Reported")
            } else if reason.range(of: "Naive Bayes") != nil {
                reasonList.append("Naive Bayes")
            } else {
                reasonList.append(reason)
            }
        }
        
        let dataDict = [
            "authorization": self.key,
            "key": self.key,
            "site": post.link?.absoluteString.components(separatedBy: "/q")[0] ?? "https://stackoverflow.com",
            "reasons": reasonList,
            "post": [
                "title": post.title,
                "body": post.body,
                "question_id": String(post.id ?? -1),
                "likelihood": String(likelihood),
                "link": post.link?.absoluteString,
                "username": post.owner?.display_name,
                "user_reputation": String(post.owner?.reputation ?? 1),
                "user_link": post.owner?.link?.absoluteString,
                "post_creation_date": creation_date
            ]
        ] as [String : Any]

        let jsonData = try JSONSerialization.data(withJSONObject: dataDict)

        let response = try client.parseJSON(try client.post(self.host + "/posts/new", data: jsonData, contentType: "application/json"))
        
        guard let json = response as? [String: Any] else {
            throw BonfireError.invalidJSON(json: response)
        }
        
        guard let statusCode = json["code"] as? String else {
            throw BonfireError.invalidJSON(json: response)
        }
        
        if statusCode != "200" {
            throw BonfireError.postCreationFailed(status: Int(statusCode)!)
        }
        
        guard let data = json["data"] as? [String: Any] else {
            throw BonfireError.invalidJSON(json: response)
        }
        
        guard let postID = data["post_id"] as? Int else {
            throw BonfireError.invalidJSON(json: response)
        }
        
        return getPostLink(postID)
    }
    
    init(key: String, client: Client, host: String = "http://localhost:3000") {
        self.key = key
        self.client = client
        self.host = host
    }
}
