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
import FireAlarmCore

class BlacklistFilter: Filter {
    let reporter: Reporter
    let troll: Bool
    
    var blacklistManager: BlacklistManager { return troll ? reporter.trollBlacklistManager : reporter.blacklistManager }
    
    required init(reporter: Reporter) {
        self.reporter = reporter
        self.troll = false
    }
    
    required init(reporter: Reporter, troll: Bool = false) {
        self.reporter = reporter
        self.troll = troll
    }
    
    
    var blacklistType: BlacklistManager.BlacklistType {
        fatalError("blacklistType must be overridden")
    }
    func content(for post: Post, site: String) -> [String?] {
        fatalError("content(for:site:) must be overridden")
    }
    
    func check(post: Post, site: String) -> FilterResult? {
        let content = self.content(for: post, site: site).compactMap { $0 }
        guard !content.isEmpty else {
            print("\(String(describing: type(of: self))): No content for \(post.id.map { String($0) } ?? "<no ID>")!")
            return nil
        }
        
        let blacklist = blacklistManager.blacklist(ofType: blacklistType)
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
    override func content(for post: Post, site: String) -> [String?] { return [post.body] }
}

class FilterBlacklistedUsername: BlacklistFilter {
    override var blacklistType: BlacklistManager.BlacklistType { return .username }
    override func content(for post: Post, site: String) -> [String?] { return [post.owner?.display_name] }
}

class FilterBlacklistedTag: BlacklistFilter {
    override var blacklistType: BlacklistManager.BlacklistType { return .tag }
    override func content(for post: Post, site: String) -> [String?] {
        return (post as? Question)?.tags ?? (post as? Answer)?.tags ?? []
    }
}
