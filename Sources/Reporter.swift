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

var reportedPosts = [(id: Int, when: Date, difference: Int)]()

class Reporter {
	let rooms: [ChatRoom]
	
	enum ReportResult {
		case notBad	//the post was not bad
		case alreadyClosed //the post is already closed
		case alreadyReported //the post was recently reported
		case reported(reason: PostClassifier.ReportReason)
	}
	
	enum ReportsLoadingError: Error {
		case ReportsNotArrayOfDictionaries
		case InvalidReport(report: [String:Any])
	}
	
	init(_ rooms: [ChatRoom]) {
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
				return (id: id, when: Date(timeIntervalSince1970: TimeInterval(when)), difference: difference)
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
	func report(post: Question, reason: PostClassifier.ReportReason) -> ReportResult {
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
				let title = "\(post.title ?? "<no title>")"
					.replacingOccurrences(of: "[", with: "\\[")
					.replacingOccurrences(of: "]", with: "\\]")
				
				let tags = post.tags ?? []
				
				let message = "[ [\(botName)](\(stackAppsLink)) ] " +
					"[tag:\(tags.first ?? "tagless")] \(header) [\(title)](//stackoverflow.com/q/\(id)) (filter score: \(difference))" +
					room.notificationString(tags: tags, reason: reason)
				
				room.postMessage(message)
				
			}
		}
		
		return .reported(reason: reason)
	}
}
