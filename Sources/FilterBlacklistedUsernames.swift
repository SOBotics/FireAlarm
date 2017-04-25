//
//  FilterBlacklistedUsernames.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 24/04/17.
//
//

import Foundation
import SwiftChatSE
import Dispatch
import SwiftStack

class FilterBlacklistedUsernames {
    var blacklistedUsernames: [String]
    
    enum FilterUsersLoadingError: Error {
        case UsernamesNotArrayOfStrings
    }
    
    init () {
        print ("Loading blacklisted usernames...")
        
        blacklistedUsernames = []
        
        let usernameURL = saveDirURL.appendingPathComponent("blacklisted_users.json")
        
        do {
            let usernameData = try Data(contentsOf: usernameURL)
            guard let usernames = try JSONSerialization.jsonObject(with: usernameData, options: []) as? [String] else {
                throw FilterUsersLoadingError.UsernamesNotArrayOfStrings
            }
            blacklistedUsernames = usernames
            
        } catch {
            handleError(error, "while loading blacklisted usernames")
            print("Loading an empty username database.")
            if FileManager.default.fileExists(atPath: usernameURL.path) {
                print("Backing up blacklisted_users.json.")
                do {
                    try FileManager.default.moveItem(at: usernameURL, to: saveDirURL.appendingPathComponent("blacklisted_users.json.bak"))
                } catch {
                    handleError(error, "while backing up the blacklisted usernames")
                }
            }
        }
    }
    
    func runUsernameFilter(_ post: Question) -> Bool {
        guard let name = post.owner?.display_name else {
            print("No username for \(post.id.map { String($0) } ?? "<no ID>")!")
            return false
        }
        for regex in blacklistedUsernames {
            if name.range(of: regex, options: [.regularExpression, .caseInsensitive]) != nil {
                return true
            }
        }
        
        
        return false
    }
    
    func saveUsernameBlacklist() throws {
        let data = try JSONSerialization.data(withJSONObject:blacklistedUsernames, options: .prettyPrinted)
        try data.write(to: saveDirURL.appendingPathComponent("blacklisted_users.json"))
    }
}
