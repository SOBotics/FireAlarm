//
//  Reports.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 24/04/17.
//
//

import Foundation
import SwiftStack
import SwiftChatSE
import Dispatch

struct Report {
    let id: Int
    let when: Date
    let difference: Int?
    let likelihood: Int
    
    let messages: [(host: ChatRoom.Host, roomID: Int, messageID: Int)]
    let details: String?
    
    init(
        id: Int,
        when: Date,
        likelihood: Int,
        difference: Int?,
        messages: [(host: ChatRoom.Host, roomID: Int, messageID: Int)] = [],
        details: String? = nil
        ) {
        
        self.id = id
        self.when = when
        self.likelihood = likelihood
        self.difference = difference
        self.messages = messages
        self.details = details
    }
    
    init?(json: [String:Any]) {
        guard let id = json["id"] as? Int, let when = json["t"] as? Int else {
            return nil
        }
        
        let messages = (json["m"] as? [[String:Any]])?.flatMap { messageJSON in
            guard let host = (messageJSON["h"] as? Int).map ({ChatRoom.Host(rawValue: $0)}) ?? nil,
                let room = messageJSON["r"] as? Int,
                let message = messageJSON["m"] as? Int else {
                    return nil
            }
            
            return (host: host, roomID: room, messageID: message)
            } as [(host: ChatRoom.Host, roomID: Int, messageID: Int)]? ?? []
        
        let likelihood: Int
        
        if let l = json["l"] as? Int {
            likelihood = l
        } else {
            likelihood = (json["d"] as? Int) ?? -1
        }
        
        let why = json["w"] as? String
        
        self.init(
            id: id,
            when: Date(timeIntervalSince1970: TimeInterval(when)),
            likelihood: likelihood,
            difference: (json["d"] as? Int),
            messages: messages,
            details: why
        )
    }
    
    var json: [String:Any] {
        var result = [String:Any]()
        result["id"] = id
        result["t"] = Int(when.timeIntervalSince1970)
        result["l"] = likelihood
        if let d = difference {
            result["d"] = d
        }
        
        if let w = details {
            result["w"] = w
        }
        
        
        result["m"] = messages.map {
            ["h":$0.host.rawValue, "r":$0.roomID, "m":$0.messageID]
        }
        
        return result
    }
}

var reportedPosts = [Report]()

class Reporter {
    var postFetcher: PostFetcher!
    let rooms: [ChatRoom]
    let trollRooms: [ChatRoom]
    
    enum TrollSites {
        case sites([Site])
        case all
        
        mutating func add(_ site: Site) {
            switch self {
            case .all: break
            case .sites(let sites): self = .sites(sites + [site])
            }
        }
        
        func contains(_ site: Site) -> Bool {
            switch self {
            case .all: return true
            case .sites(let sites): return sites.contains(site)
            }
        }
    }
    
    var trollFilters = [Filter]()
    var trollSites: TrollSites = .sites([])
    
    var staticDB: DatabaseConnection
    
    var filters = [Filter]()
    
    var blacklistManager: BlacklistManager
    var trollBlacklistManager: BlacklistManager
    
    private let queue = DispatchQueue(label: "Reporter queue")
    
    
    func filter<T: Filter>(ofType type: T.Type) -> T? {
        for filter in filters {
            if let f = filter as? T {
                return f
            }
        }
        return nil
    }
    
    init(rooms: [ChatRoom], trollRooms: [ChatRoom] = []) {
        print ("Reporter loading...")
        
        self.rooms = rooms
        self.trollRooms = trollRooms
        
        let blacklistURL = saveDirURL.appendingPathComponent("blacklists.json")
        trollBlacklistManager = BlacklistManager()
        do {
            blacklistManager = try BlacklistManager(url: blacklistURL)
        } catch {
            handleError(error, "while loading blacklists")
            print("Loading an empty blacklist.")
            blacklistManager = BlacklistManager()
            if FileManager.default.fileExists(atPath: blacklistURL.path) {
                print("Backing up blacklists.json.")
                do {
                    try FileManager.default.moveItem(at: blacklistURL, to: saveDirURL.appendingPathComponent("blacklist.json.bak"))
                } catch {
                    handleError(error, "while backing up the blacklists")
                }
            }
        }
        
        let reportsURL = saveDirURL.appendingPathComponent("reports.json")
        let usernameURL = saveDirURL.appendingPathComponent("blacklisted_users.json")
        do {
            let reportData = try Data(contentsOf: reportsURL)
            guard let reports = try JSONSerialization.jsonObject(with: reportData, options: []) as? [[String:Any]] else {
                throw ReportsLoadingError.ReportsNotArrayOfDictionaries
            }
            
            reportedPosts = try reports.map {
                guard let report = Report(json: $0) else {
                    throw ReportsLoadingError.InvalidReport(report: $0)
                }
                return report
            }
            
        } catch {
            handleError(error, "while loading reports")
            print("Loading an empty report list.")
            if FileManager.default.fileExists(atPath: reportsURL.path) {
                print("Backing up reports.json.")
                do {
                    try FileManager.default.moveItem(at: usernameURL, to: saveDirURL.appendingPathComponent("reports.json.bak"))
                } catch {
                    handleError(error, "while backing up the reports")
                }
            }
        }
        
        do {
            staticDB = try DatabaseConnection("filter_static.sqlite")
        } catch {
            fatalError("Could not load filter_static.sqlite:\n\(error)")
        }
        
        filters = [
            FilterNaiveBayes(reporter: self),
            FilterMisleadingLinks(reporter: self),
            FilterBlacklistedKeyword(reporter: self),
            FilterBlacklistedUsername(reporter: self),
            FilterBlacklistedTag(reporter: self),
            FilterImageWithoutCode(reporter: self),
            FilterLowLength(reporter: self)
        ]
        trollFilters = [
            FilterBlacklistedKeyword(reporter: self, troll: true),
            FilterBlacklistedUsername(reporter: self, troll: true),
            FilterBlacklistedTag(reporter: self, troll: true)
        ]
        
        postFetcher = PostFetcher(rooms: rooms, reporter: self, staticDB: staticDB)
    }
    
    func checkPost(_ post: Post, site: Site) throws -> [FilterResult] {
        let filters = trollSites.contains(site) ? trollFilters : self.filters
        return try filters.flatMap { try $0.check(post, site: site) }
    }
    
    @discardableResult func checkAndReportPost(_ post: Post, site: Site) throws -> ReportResult {
        let results = try checkPost(post, site: site)
        
        return try report(post: post, site: site, reasons: results)
    }
    
    struct ReportResult {
        enum Status {
            case notBad	//the post was not bad
            case alreadyClosed //the post is already closed
            case alreadyReported //the post was recently reported
            case reported
        }
        var status: Status
        var filterResults: [FilterResult]
    }
    
    enum ReportsLoadingError: Error {
        case ReportsNotArrayOfDictionaries
        case InvalidReport(report: [String:Any])
    }
    
    func saveReports() throws {
        let data = try JSONSerialization.data(
            withJSONObject: reportedPosts.map { $0.json }
        )
        
        try data.write(to: saveDirURL.appendingPathComponent("reports.json"))
    }
    
    enum ReportError: Error {
        case missingSite(id: Int)
    }
    
    ///Reports a post if it has not been recently reported.  Returns either .reported or .alreadyReported.
    func report(post: Post, site: Site, reasons: [FilterResult]) throws -> ReportResult {
        var status: ReportResult.Status = .notBad
        
        queue.sync {
            guard let id = post.id else {
                print("No post ID!")
                status = .notBad
                return
            }
            
            let isManualReport = reasons.contains {
                if case .manuallyReported = $0.type {
                    return true
                } else {
                    return false
                }
            }
            
            if !isManualReport && reportedPosts.lazy.reversed().contains(where: { $0.id == id }) {
                print("Not reporting \(id) because it was recently reported.")
                status = .alreadyReported
                return
            }
            
            /*if !isManualReport && post.closed_reason != nil {
             print ("Not reporting \(post.id ?? 0) as it is closed.")
             status = .alreadyClosed
             return
             }*/
            
            var reported = false
            var postDetails = "Details unknown."
            var bayesianDifference: Int?

            var title = "\(post.title ?? "<no title>")"
                .replacingOccurrences(of: "[", with: "\\[")
                .replacingOccurrences(of: "]", with: "\\]")
            
            while title.hasSuffix("\\") {
                title = String(title.dropLast())
            }
            
            let tags = (post as? Question)?.tags ?? []
            postDetails = reasons.map {$0.details ?? "Details unknown."}.joined (separator: ", ")
            
            var messages: [(host: ChatRoom.Host, roomID: Int, messageID: Int)] = []
            
            let sema = DispatchSemaphore(value: 0)
            
            let rooms: [ChatRoom] = trollSites.contains(site) ? self.trollRooms : self.rooms
            var bonfireLink: String?
            
            //Post weight including custom filter weight subtracted from Naive Bayes difference.
            var combinedPostWeight = 0
            for reason in reasons {
                if case .bayesianFilter(let difference) = reason.type {
                    bayesianDifference = difference
                    combinedPostWeight += difference
                } else if case .customFilterWithWeight(_, let weight) = reason.type {
                    combinedPostWeight -= weight
                }
            }
            
            for room in rooms {
                //Filter out weights which are less than this room's threshold.
                let reasons = reasons.filter {
                    if case .bayesianFilter(_) = $0.type {
                        return combinedPostWeight < room.thresholds[site.id] ?? Int.min
                    } else if case .customFilterWithWeight(_, _) = $0.type {
                        return combinedPostWeight < room.thresholds[site.id] ?? Int.min
                    }
                    return true
                }
                
                if reasons.isEmpty {
                    sema.signal()
                    continue
                }
                
                reported = true
                
                if bonfireLink == nil {
                    do {
                        bonfireLink = try bonfire?.uploadPost(post: post, postDetails: postDetails, likelihood: combinedPostWeight)
                    } catch {
                        print("Could not upload the post to Bonfire!")
                        print(error)
                    }
                }
                
                let header = reasons.map { $0.header }.joined(separator: ", ")
                let message: String
                
                if let bonfireLink = bonfireLink {
                    message = "[ [\(botName)](\(stackAppsLink)) | [Bonfire](\(bonfireLink)) ] " +
                        "[tag:\(tags.first ?? "tagless")] \(header) [\(title)](//\(site.domain)/q/\(id)) " +
                        room.notificationString(tags: tags, reasons: reasons)
                } else {
                    message = "[ [\(botName)](\(stackAppsLink)) ] " +
                        "[tag:\(tags.first ?? "tagless")] \(header) [\(title)](//\(site.domain)/q/\(id)) " +
                        room.notificationString(tags: tags, reasons: reasons)
                }
                
                room.postMessage(message, completion: {message in
                    if let message = message {
                        messages.append((host: room.host, roomID: room.roomID, messageID: message))
                    }
                    sema.signal()
                })
            }
            rooms.forEach { _ in sema.wait() }
            
            
            if reported {
                let report = Report(
                    id: id,
                    when: Date(),
                    likelihood: combinedPostWeight,
                    difference: bayesianDifference,
                    messages: messages,
                    details: postDetails
                )
                
                reportedPosts.append(report)
                
                status = .reported
                return
            } else {
                status = .notBad
                return
            }
        }
        
        return ReportResult(status: status, filterResults: reasons)
    }
}
