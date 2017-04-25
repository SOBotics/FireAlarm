//
//  PostClassifier.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 24/04/17.
//
//

import Foundation
import SwiftChatSE
import SwiftStack
import Dispatch

class PostClassifier {
    
    var filterNaiveBayes: FilterNaiveBayes!
    var filterMisleadingLinks: FilterMisleadingLinks!
    var filterBlacklistedUsernames: FilterBlacklistedUsernames!
    
    let rooms: [ChatRoom]
    
    init(_ rooms: [ChatRoom]) {
        print ("Post Classifier loading...")
        
        self.rooms = rooms
        
        filterNaiveBayes = FilterNaiveBayes()
        filterMisleadingLinks = FilterMisleadingLinks()
        filterBlacklistedUsernames = FilterBlacklistedUsernames()
    }
    
    func checkPost(_ post: Question) -> ReportReason? {
        let bayesianResults = filterNaiveBayes.runBayesianFilter(post)
        
        if filterMisleadingLinks.runLinkFilter(post) {
            return .misleadingLink
        } else if filterBlacklistedUsernames.runUsernameFilter(post) {
            return .blacklistedUsername
        } else if rooms.contains(where: {room in return bayesianResults < room.threshold}) {
            return .bayesianFilter(difference: bayesianResults)
        } else {
            return nil
        }
    }
    
    enum ReportReason {
        case bayesianFilter(difference: Int)
        case blacklistedUsername
        case misleadingLink
        case manuallyReported
    }
    
    enum ReportResult {
        case notBad	//the post was not bad
        case alreadyClosed //the post is already closed
        case alreadyReported //the post was recently reported
        case reported(reason: ReportReason)
    }
    
    @discardableResult func checkAndReportPost(_ post: Question) throws -> ReportResult {
        if let reason = checkPost(post) {
            if (post.closed_reason == nil) {
                return report(post: post, reason: reason)
            } else {
                print ("Not reporting \(post.id ?? 0) as it is closed.")
                return .alreadyClosed
            }
        }
        else {
            if (post.id ?? 1) % 10000 == 0 && post.last_edit_date == nil {
                rooms.first!.postMessage("[ [\(botName)](\(stackAppsLink)) ] " +
                    "[tag:\(tags(for: post).first ?? "tagless")] Potentially bad question: " +
                    "[\(post.title ?? "<no title>")](//youtube.com/watch?v=dQw4w9WgXcQ)"
                )
            }
        }
        return .notBad
    }
    
    func tags(for post: Question) -> [String] {
        return post.tags ?? []
    }
    
    ///Reports a post if it has not been recently reported.  Returns either .reported or .alreadyReported.
    func report(post: Question, reason: ReportReason) -> ReportResult {
        guard let id = post.id else {
            print("No post ID!")
            return .notBad
        }
        
        
        if let minDate: Date = Calendar(identifier: .gregorian).date(byAdding: DateComponents(hour: -6), to: Date()) {
            let recentlyReportedPosts = reportedPosts.filter {
                $0.when > minDate
            }
            if recentlyReportedPosts.contains(where: { $0.id == id }) {
                print("Not reporting \(id) because it was recently reported.")
                return .alreadyReported
            }
        }
        else {
            rooms.forEach {$0.postMessage("Failed to calculate minimum report date!")}
        }
        
        
        print("Reporting question \(id).")
        
        let header: String
        var difference: Int = 0
        switch reason {
        case .bayesianFilter(let d):
            difference = d
            header = "Potentially bad question:"
        case .blacklistedUsername:
            header = "Blacklisted username:"
        case .misleadingLink:
            header = "Misleading link:"
        case .manuallyReported:
            header = "Manually reported question:"
        }
        
        reportedPosts.append((id: id, when: Date(), difference: difference))
        
        for room in rooms {
            if difference < room.threshold {
                var newTitle = "\(post.title ?? "<no title>")"
                
                newTitle = newTitle.replacingOccurrences(of: "[", with: "\\[")
                newTitle = newTitle.replacingOccurrences(of: "]", with: "\\]")
                
                let message = "[ [\(botName)](\(stackAppsLink)) ] " +
                    "[tag:\(tags(for: post).first ?? "tagless")] \(header) [\(newTitle)](//stackoverflow.com/q/\(id)) (filter score: \(difference))" +
                    room.notificationString(tags: tags(for: post), reason: reason)
                
                room.postMessage(message)
                
            }
        }
        
        return .reported(reason: reason)
    }
}
