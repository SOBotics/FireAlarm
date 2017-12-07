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

class BlacklistFilter: Filter {
    let reporter: Reporter
    
    required init(reporter: Reporter) {
        self.reporter = reporter
    }
    
    
    var blacklistType: BlacklistManager.BlacklistType {
        fatalError("blacklistType must be overridden")
    }
    func content(for post: Question, site: Site) -> [String?] {
        fatalError("content(for:site:) must be overridden")
    }
    
    func check(_ post: Question, site: Site) -> FilterResult? {
        let content = self.content(for: post, site: site).flatMap { $0 }
        guard !content.isEmpty else {
            print("\(String(describing: type(of: self))): No content for \(post.id.map { String($0) } ?? "<no ID>")!")
            return nil
        }
        
        let blacklist = reporter.blacklistManager.blacklist(ofType: blacklistType)
        let matches = content.map { blacklist.items(catching: $0) }.joined()
        let uppercasedBlacklistName = String(blacklistType.rawValue.first!).uppercased() + blacklistType.rawValue.dropFirst()
        
        if matches.count == 0 {
            return nil
        } else {
            let details: String
            if matches.count == 1 {
                details = "\(uppercasedBlacklistName) matched regex `\(matches.first!)`"
            } else {
                details = "\(uppercasedBlacklistName)s matched regular expressions \(formatArray(matches.map { "`\($0)`" }, conjunction: "and"))"
            }
            return FilterResult(
                type: .customFilter(filter: self),
                header: "Blacklisted \(blacklistType.rawValue)",
                details: details
            )
        }
    }
    
    func save() throws {
        
    }
}


class FilterBlacklistedKeyword: BlacklistFilter {
    override var blacklistType: BlacklistManager.BlacklistType { return .keyword }
    override func content(for post: Question, site: Site) -> [String?] { return [post.body] }
}

class FilterBlacklistedUsername: BlacklistFilter {
    override var blacklistType: BlacklistManager.BlacklistType { return .username }
    override func content(for post: Question, site: Site) -> [String?] { return [post.owner?.display_name] }
}

class FilterBlacklistedTag: BlacklistFilter {
    override var blacklistType: BlacklistManager.BlacklistType { return .tag }
    override func content(for post: Question, site: Site) -> [String?] { return post.tags ?? [] }
}
