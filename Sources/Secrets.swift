//
//  Secrets.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 6/8/17.
//
//

import Foundation

struct Secrets {
    let email: String?
    let password: String?
    let githubWebhookSecret: String?
    
    init?(json: [String:String]) {
        email = json["email"]
        password = json["password"]
        githubWebhookSecret = json["githubWebhookSecret"]
    }
}

var secrets: Secrets!
