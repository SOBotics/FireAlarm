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
	
	let messages: [(host: ChatRoom.Host, roomID: Int, messageID: Int)]
	let details: String?
	
	
	init(
		id: Int,
		when: Date,
		difference: Int?,
		messages: [(host: ChatRoom.Host, roomID: Int, messageID: Int)] = [],
		details: String? = nil
		) {
		
		self.id = id
		self.when = when
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
		let why = json["w"] as? String
		
		self.init(
			id: id,
			when: Date(timeIntervalSince1970: TimeInterval(when)),
			difference: (json["d"] as? Int),
			messages: messages,
			details: why
		)
	}
	
	var json: [String:Any] {
		var result = [String:Any]()
		result["id"] = id
		result["t"] = Int(when.timeIntervalSince1970)
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
        //Debug print
        print ("Checking post id \(post.id ?? -1).")
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
			withJSONObject: reportedPosts.map { $0.json }
		)
		
		try data.write(to: saveDirURL.appendingPathComponent("reports.json"))
	}
	
	///Reports a post if it has not been recently reported.  Returns either .reported or .alreadyReported.
	func report(post: Question, reasons: [FilterResult]) -> ReportResult {
		guard let id = post.id else {
			print("No post ID!")
			return .notBad
		}
        
        //Debug print
        print ("'report' called on post \(id)")
		
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
		
		var reported = false
		
		var bayesianDifference: Int?
		
		var postDetails = "Details unknown."
		
		
		
		let title = "\(post.title ?? "<no title>")"
			.replacingOccurrences(of: "[", with: "\\[")
			.replacingOccurrences(of: "]", with: "\\]")
		
		let tags = post.tags ?? []
		
		let header = reasons.map { $0.header }.joined(separator: ", ")
		
		postDetails = reasons.map {$0.details ?? "Details unknown."}.joined (separator: ", ")
		
		
		
		var messages: [(host: ChatRoom.Host, roomID: Int, messageID: Int)] = []
		
		
		
		let sema = DispatchSemaphore(value: 0)
		
		for room in rooms {
			//Filter out Bayesian scores which are less than this room's threshold.
			let reasons = reasons.filter {
				if case .bayesianFilter(let difference) = $0.type {
					bayesianDifference = difference
					return difference < room.threshold
				}
				return true
			}
			if reasons.isEmpty {
				sema.signal()
				continue
			}
			
			reported = true
			
			
			let message = "[ [\(botName)](\(stackAppsLink)) ] " +
				"[tag:\(tags.first ?? "tagless")] \(header) [\(title)](//stackoverflow.com/q/\(id)) " +
				room.notificationString(tags: tags, reasons: reasons)
			
			room.postMessage(message, completion: {message in
				if let message = message {
					messages.append((host: room.host, roomID: room.roomID, messageID: message))
				}
				sema.signal()
			})
		}
		rooms.forEach { _ in sema.wait() }
		
		
		if reported {
			reportedPosts.append(Report(
				id: id,
				when: Date(),
				difference: bayesianDifference,
				messages: messages,
				details: postDetails
				)
			)
			
			return .reported(reasons: reasons)
		} else {
			return .notBad
		}
	}
}
