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

var reportedPosts = [(id: Int, when: Date, difference: Int, messageID: Int, details: String)]()

class Reporter {
	var postFetcher: PostFetcher!
	let rooms: [ChatRoom]
	
	var filters = [Filter]()
	
	func filter<T: Filter>(ofType type: T.Type) -> T? {
		for filter in filters {
			if let f = filter as? T {
				return f
			}
		}
		return nil
	}
	
	init(_ rooms: [ChatRoom]) {
		print ("Reporter loading...")
		
		self.rooms = rooms
		
		let reportsURL = saveDirURL.appendingPathComponent("reports.json")
		let usernameURL = saveDirURL.appendingPathComponent("blacklisted_users.json")
		do {
			let reportData = try Data(contentsOf: reportsURL)
			guard let reports = try JSONSerialization.jsonObject(with: reportData, options: []) as? [[String:Any]] else {
				throw ReportsLoadingError.ReportsNotArrayOfDictionaries
			}
			
			reportedPosts = try reports.map {
				guard let id = $0["id"] as? Int, let when = $0["t"] as? Int else {
					throw ReportsLoadingError.InvalidReport(report: $0)
				}
				let difference = ($0["d"] as? Int) ?? 0
                return (id: id, when: Date(timeIntervalSince1970: TimeInterval(when)), difference: difference, messageID: -1, details: "Details for this post are lost.")
			}
			
		} catch {
			handleError(error, "while loading reports")
			print("Loading an empty report list.")
			if FileManager.default.fileExists(atPath: reportsURL.path) {
				print("Backing up reports.json.")
				do {
					try FileManager.default.moveItem(at: usernameURL, to: saveDirURL.appendingPathComponent("reports.json.bak"))
				} catch {
					handleError(error, "while backing up the blacklisted usernames")
				}
			}
		}
		
		filters = [
			FilterNaiveBayes(),
			FilterMisleadingLinks(),
			FilterBlacklistedUsernames()
		]
		
		postFetcher = PostFetcher(rooms: rooms, reporter: self)
	}
	
	func checkPost(_ post: Question) -> [FilterResult] {
		return filters.flatMap { $0.check(post) }
	}
	
	@discardableResult func checkAndReportPost(_ post: Question) throws -> Reporter.ReportResult {
		let results = checkPost(post)
		
		return report(post: post, reasons: results)
	}
	
	enum ReportResult {
		case notBad	//the post was not bad
		case alreadyClosed //the post is already closed
		case alreadyReported //the post was recently reported
		case reported(reasons: [FilterResult])
	}
	
	enum ReportsLoadingError: Error {
		case ReportsNotArrayOfDictionaries
		case InvalidReport(report: [String:Any])
	}
	
	func saveReports() throws {
		let data = try JSONSerialization.data(
			withJSONObject: reportedPosts.map {
				["id":$0.id,"t":Int($0.when.timeIntervalSince1970),"d":$0.difference]
			}
		)
		
		try data.write(to: saveDirURL.appendingPathComponent("reports.json"))
	}
	
	
	
	///Reports a post if it has not been recently reported.  Returns either .reported or .alreadyReported.
	func report(post: Question, reasons: [FilterResult]) -> ReportResult {
		guard let id = post.id else {
			print("No post ID!")
			return .notBad
		}
        
        var manuallyReported = false
        
        let _ = reasons.filter {
            if case .manuallyReported = $0.type {
                manuallyReported = true
                return true
            }
            return false
        }
		
        if manuallyReported == false {
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
            
            if (post.closed_reason != nil) {
                print ("Not reporting \(post.id ?? 0) as it is closed.")
                return .alreadyClosed
            }
        }
		
		var reported = false
		
		var bayesianDifference: Int?
        
        var idMessage = -1
        
        var postDetails = "Details unknown."
		
		for room in rooms {
			//Filter out Bayesian scores which are less than this room's threshold.
			let reasons = reasons.filter {
				if case .bayesianFilter(let difference) = $0.type {
					bayesianDifference = difference
					return difference < room.threshold
				}
				return true
			}
			if reasons.isEmpty { continue }
			reported = true
			
			let title = "\(post.title ?? "<no title>")"
				.replacingOccurrences(of: "[", with: "\\[")
				.replacingOccurrences(of: "]", with: "\\]")
			
			let tags = post.tags ?? []
			
			let header = reasons.map { $0.header }.joined(separator: ", ")
            
            postDetails = reasons.map {$0.details ?? "Details unknown."}.joined (separator: ", ")
			
			let message = "[ [\(botName)](\(stackAppsLink)) ] " +
				"[tag:\(tags.first ?? "tagless")] \(header) [\(title)](//stackoverflow.com/q/\(id)) " +
				room.notificationString(tags: tags, reasons: reasons)
			
            room.postMessage(message, completion: {
                messageID in
                idMessage = messageID
                print ("\(messageID)   ||  \(idMessage)")
            })
		}
		
		if reported {
            reportedPosts.append((id: id, when: Date(), difference: bayesianDifference ?? 0, messageID: idMessage, details: postDetails))
            print (" ID MESSAGE:\(idMessage)")
			return .reported(reasons: reasons)
		} else {
			return .notBad
		}
	}
}
