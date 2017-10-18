//
//  FilterBlacklistedKeywords.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/15/17.
//  Copyright © 2017 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE
import Dispatch
import SwiftStack

class FilterBlacklistedKeywords: Filter {
    var blacklistedKeywords: [String]
    
    enum LoadingError: Error {
        case KeywordsNotArrayOfStrings
    }
    
    required init(reporter: Reporter) {
        print ("Loading blacklisted keywords...")
        
        blacklistedKeywords = []
        
        let keywordURL = saveDirURL.appendingPathComponent("blacklisted_keywords.json")
        
        do {
            let keywordData = try Data(contentsOf: keywordURL)
            guard let keywords = try JSONSerialization.jsonObject(with: keywordData, options: []) as? [String] else {
                throw LoadingError.KeywordsNotArrayOfStrings
            }
            blacklistedKeywords = keywords
            
        } catch {
            handleError(error, "while loading blacklisted keywords")
            print("Loading an empty keyword database.")
            if FileManager.default.fileExists(atPath: keywordURL.path) {
                print("Backing up blacklisted_users.json.")
                do {
                    try FileManager.default.moveItem(at: keywordURL, to: saveDirURL.appendingPathComponent("blacklisted_keywords.json.bak"))
                } catch {
                    handleError(error, "while backing up the blacklisted keywords")
                }
            }
        }
    }
    
    func check(_ post: Question, site: Site) -> FilterResult? {
        guard let content = post.body else {
            print("No body for \(post.id.map { String($0) } ?? "<no ID>")!")
            return nil
        }
        for regex in blacklistedKeywords {
            if content.range(of: regex, options: [.regularExpression, .caseInsensitive]) != nil {
                return FilterResult(
                    type: .customFilter(filter: self),
                    header: "Blacklisted keyword",
                    details: "Keyword matched regex `\(regex)`"
                )
            }
        }
        
        
        return nil
    }
    
    func save() throws {
        let data = try JSONSerialization.data(withJSONObject:blacklistedKeywords, options: .prettyPrinted)
        try data.write(to: saveDirURL.appendingPathComponent("blacklisted_keywords.json"))
    }
}
