//
//  CommandBlacklist.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/15/17.
//
//

import Foundation
import SwiftChatSE

class TrollCommandBlacklist: CommandBlacklist {
    override class func blacklistManager(reporter: Reporter) -> BlacklistManager {
        return reporter.trollBlacklistManager
    }
}

class TrollCommandGetBlacklist: CommandGetBlacklist {
    override class func blacklistManager(reporter: Reporter) -> BlacklistManager {
        return reporter.trollBlacklistManager
    }
}

class TrollCommandUnblacklist: CommandUnblacklist {
    override class func blacklistManager(reporter: Reporter) -> BlacklistManager {
        return reporter.trollBlacklistManager
    }
}
