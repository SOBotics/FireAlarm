//
//  main.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 8/27/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

func clearCookies() {
    let storage = NSHTTPCookieStorage.sharedHTTPCookieStorage()
    if let cookies = storage.cookies {
        for cookie in cookies {
            storage.deleteCookie(cookie)
        }
    }
}

print("FireAlarm starting...")

let client = Client(host: .StackOverflow)

if !client.loggedIn {
    let email: String
    let password: String
    
    let env =  NSProcessInfo.processInfo().environment
    
    let envEmail = env["ChatBotEmail"]
    let envPassword = env["ChatBotPass"]
    
    if envEmail != nil {
        email = envEmail!
    }
    else {
        print("Email: ", terminator: "")
        email = readLine()!
    }
    
    if envPassword != nil {
        password = envPassword!
    }
    else {
        password = String(UTF8String: getpass("Password: "))!
    }
    
    do {
        try client.loginWithEmail(email, password: password)
    }
    catch Client.LoginError.LoginFailed(let message) {
        print("Login failed: \(message)")
        exit(EXIT_FAILURE)
    }
    catch {
        print("Login failed with error \(error).\nClearing cookies and retrying.")
        clearCookies()
        do {
            try client.loginWithEmail(email, password: password)
        }
        catch {
            print("Failed to log in!")
            exit(EXIT_FAILURE)
        }
    }
}