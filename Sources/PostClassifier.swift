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
	var postFetcher: PostFetcher!
	let reporter: Reporter
    
    var filterNaiveBayes: FilterNaiveBayes!
    var filterMisleadingLinks: FilterMisleadingLinks!
    var filterBlacklistedUsernames: FilterBlacklistedUsernames!
    
    let rooms: [ChatRoom]
	
	enum ReportReason {
		case bayesianFilter(difference: Int)
		case blacklistedUsername
		case misleadingLink
		case manuallyReported
	}
    
    init(_ rooms: [ChatRoom]) {
        print ("Post Classifier loading...")
        
        self.rooms = rooms
		reporter = Reporter(rooms)
        
        filterNaiveBayes = FilterNaiveBayes()
        filterMisleadingLinks = FilterMisleadingLinks()
        filterBlacklistedUsernames = FilterBlacklistedUsernames()
		
		
		postFetcher = PostFetcher(rooms: rooms, classifier: self)
    }
    
    func checkPost(_ post: Question) -> PostClassifier.ReportReason? {
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
    
    @discardableResult func checkAndReportPost(_ post: Question) throws -> Reporter.ReportResult {
        if let reason = checkPost(post) {
            if (post.closed_reason == nil) {
                return reporter.report(post: post, reason: reason)
            } else {
                print ("Not reporting \(post.id ?? 0) as it is closed.")
                return .alreadyClosed
            }
        }
        return .notBad
    }
}
