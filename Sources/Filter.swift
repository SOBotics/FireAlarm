//
//  Filter.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/24/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE
import SwiftStack
import Dispatch

extension Post {
	var id: Int? {
		if let q = self as? Question, let id = q.question_id {
			return id
		} else if let a = self as? Answer, let id = a.answer_id {
			return id
		} else {
			return post_id
		}
	}
}

var reportedPosts = [(id: Int, when: Date, difference: Int)]()
var filterNaiveBayes: FilterNaiveBayes!

class Filter {
	let client: Client
	let rooms: [ChatRoom]
	
	var blacklistedUsernames: [String]
	
	var postsToCheck = [Int]()
	
	var queue = DispatchQueue(label: "Filter", attributes: [.concurrent])
	
	
	private var lastEventDate: Date?
	
	
	enum FilterLoadingError: Error {
		case UsernamesNotArrayOfStrings
		case ReportsNotArrayOfDictionaries
		case InvalidReport(report: [String:Any])
	}
	
	init(_ rooms: [ChatRoom]) {
		client = rooms.first!.client
		self.rooms = rooms
		
		print("Loading filter...")
		blacklistedUsernames = []
		
		filterNaiveBayes = FilterNaiveBayes ()
		
		let usernameURL = saveDirURL.appendingPathComponent("blacklisted_users.json")
		
		do {
			let usernameData = try Data(contentsOf: usernameURL)
			guard let usernames = try JSONSerialization.jsonObject(with: usernameData, options: []) as? [String] else {
				throw FilterLoadingError.UsernamesNotArrayOfStrings
			}
			blacklistedUsernames = usernames
			
		} catch {
			handleError(error, "while loading blacklisted usernames")
			print("Loading an empty username database.")
			if FileManager.default.fileExists(atPath: usernameURL.path) {
				print("Backing up blacklisted_users.json.")
				do {
					try FileManager.default.moveItem(at: usernameURL, to: saveDirURL.appendingPathComponent("blacklisted_users.json.bak"))
				} catch {
					handleError(error, "while backing up the blacklisted usernames")
				}
			}
		}
		
		let reportsURL = saveDirURL.appendingPathComponent("reports.json")
		do {
			let reportData = try Data(contentsOf: reportsURL)
			guard let reports = try JSONSerialization.jsonObject(with: reportData, options: []) as? [[String:Any]] else {
				throw FilterLoadingError.ReportsNotArrayOfDictionaries
			}
			
			reportedPosts = try reports.map {
				guard let id = $0["id"] as? Int, let when = $0["t"] as? Int else {
					throw FilterLoadingError.InvalidReport(report: $0)
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
		
		
		print("Filter loaded.")
	}
	
	
	var ws: WebSocket!
	
	fileprivate var wsRetries = 0
	fileprivate let wsMaxRetries = 10
	
	private(set) var running = false
	
	func start() throws {
		running = true
		
		//let request = URLRequest(url: URL(string: "ws://qa.sockets.stackexchange.com/")!)
		//ws = WebSocket(request: request)
		//ws.eventQueue = room.client.queue
		//ws.delegate = self
		//ws.open()
		ws = try WebSocket("wss://qa.sockets.stackexchange.com/")
		
		ws.onOpen {socket in
			self.webSocketOpen()
		}
		ws.onText {socket, text in
			self.webSocketMessageText(text)
		}
		ws.onBinary {socket, data in
			self.webSocketMessageData(data)
		}
		ws.onClose {socket in
			self.webSocketClose(0, reason: "", wasClean: true)
			self.webSocketEnd(0, reason: "", wasClean: true, error: socket.error)
		}
		ws.onError {socket in
			self.webSocketEnd(0, reason: "", wasClean: true, error: socket.error)
		}
		
		try ws.connect()
		
		doCheckPosts()
	}
	
	func stop() {
		running = false
		ws?.disconnect()
	}
	
	func webSocketOpen() {
		print("Listening to active questions!")
		wsRetries = 0
		ws.write("155-questions-active")
		
		queue.async {
			while true {
				sleep(600)
				if (Date().timeIntervalSinceReferenceDate -
					(self.lastEventDate?.timeIntervalSinceReferenceDate ?? 0)) > 600 {
					
					self.ws?.disconnect()
				}
				return
			}
		}
	}
	
	func webSocketClose(_ code: Int, reason: String, wasClean: Bool) {
		//do nothing -- we'll handle this in webSocketEnd
	}
	
	func webSocketError(_ error: NSError) {
		//do nothing -- we'll handle this in webSocketEnd
	}
	
	enum QuestionProcessingError: Error {
		case textNotUTF8(text: String)
		
		case jsonNotDictionary(json: String)
		case jsonParsingError(json: String, error: String)
		case noDataObject(json: String)
		case noQuestionID(json: String)
		case noSite(json: String)
	}
	
	func runUsernameFilter(_ post: Question) -> Bool {
		guard let name = post.owner?.display_name else {
			print("No username for \(post.id.map { String($0) } ?? "<no ID>")!")
			return false
		}
		for regex in blacklistedUsernames {
			if name.range(of: regex, options: [.regularExpression, .caseInsensitive]) != nil {
				return true
			}
		}
		
		
		return false
	}
	
	func runLinkFilter(_ post: Question) -> Bool {
		do {
			let regex = try NSRegularExpression(pattern:
				"<a href=\"([^\"]*)\" rel=\"nofollow(?: noreferrer)?\">\\s*([^<\\s]*)(?=\\s*</a>)", options: []
			)
			
			guard let body = post.body else {
				print("No body for \(post.id.map { String($0) } ?? "<no ID>")!")
				return false
			}
			
			#if os(Linux)
				let nsString = body._bridgeToObjectiveC()
			#else
				let nsString = body as NSString
			#endif
			for match in regex.matches(in: body, options: [], range: NSMakeRange(0, nsString.length)) {
				
				
				#if os(Linux)
					let linkString = nsString.substring(with: match.range(at: 1))
					let textString = nsString.substring(with: match.range(at: 2))
				#else
					
					let linkString = nsString.substring(with: match.rangeAt(1)) as String
					let textString = nsString.substring(with: match.rangeAt(2)) as String
				#endif
				guard
					let link = URL(string: linkString),
					let text = URL(string: textString),
					let linkHost = link.host?.lowercased(),
					let textHost = text.host?.lowercased() else {
						continue
				}
				
				
				if (!textHost.isEmpty &&
					textHost != linkHost &&
					!linkHost.contains("rads.stackoverflow.com") &&
					"www." + textHost != linkHost &&
					"www." + linkHost != textHost &&
					linkHost.contains(".") &&
					textHost.contains(".") &&
					!linkHost.trimmingCharacters(in: .whitespaces).contains(" ") &&
					!textHost.trimmingCharacters(in: .whitespaces).contains(" ") &&
					!linkHost.contains("//http") &&
					!textHost.contains("//http")) {
					
					return true
				}
				
				
			}
			return false
			
		} catch {
			handleError(error, "while checking for misleading links")
			return false
		}
	}
	
	func checkPost(_ post: Question) -> ReportReason? {
		let bayesianResults = filterNaiveBayes.runBayesianFilter(post)
		
		if runLinkFilter(post) {
			return .misleadingLink
		} else if runUsernameFilter(post) {
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
	
	func saveUsernameBlacklist() throws {
		let data = try JSONSerialization.data(withJSONObject: blacklistedUsernames, options: .prettyPrinted)
		try data.write(to: saveDirURL.appendingPathComponent("blacklisted_users.json"))
	}
	
	func saveReports() throws {
		let data = try JSONSerialization.data(
			withJSONObject: reportedPosts.map {
				["id":$0.id,"t":Int($0.when.timeIntervalSince1970),"d":$0.difference]
			}
		)
		
		try data.write(to: saveDirURL.appendingPathComponent("reports.json"))
	}
	
	func webSocketMessageText(_ text: String) {
		do {
			guard let data = text.data(using: .utf8) else {
				throw QuestionProcessingError.textNotUTF8(text: text)
			}
			webSocketMessageData(data)
		} catch {
			handleError(error, "while processing an active question")
		}
	}
	
	func webSocketMessageData(_ data: Data) {
		let string = String(data: data, encoding: .utf8) ?? "<not UTF-8: \(data.base64EncodedString())>"
		do {
			
			do {
				guard let json = try JSONSerialization.jsonObject(with: data, options: []) as? [String:String] else {
					throw QuestionProcessingError.jsonNotDictionary(json: string)
				}
				
				guard json["action"] == "155-questions-active" else {
					if json["action"] == "hb" {
						//heartbeat
						ws.write("{\"action\":\"hb\",\"data\":\"hb\"}")
					}
					return
				}
				
				guard let dataObject = json["data"]?.data(using: .utf8) else {
					throw QuestionProcessingError.noDataObject(json: string)
				}
				
				guard let data = try JSONSerialization.jsonObject(with: dataObject, options: []) as? [String:Any] else {
					throw QuestionProcessingError.noDataObject(json: string)
				}
				
				guard let site = data["apiSiteParameter"] as? String else {
					throw QuestionProcessingError.noSite(json: string)
				}
				
				guard site == "stackoverflow" else {
					return
				}
				
				guard let id = data["id"] as? Int else {
					throw QuestionProcessingError.noQuestionID(json: string)
				}
				
				
				postsToCheck.append(id)
				//print("Another post has been recieved.  There are now \(postsToCheck.count) posts to check.")
				
			} catch {
				if let e = errorAsNSError(error) {
					throw QuestionProcessingError.jsonParsingError(json: string, error: formatNSError(e))
				} else {
					throw QuestionProcessingError.jsonParsingError(json: string, error: String(describing: error))
				}
			}
		}
		catch {
			handleError(error, "while processing an active question")
		}
	}
	
	private func doCheckPosts() {
		queue.async {
			while true {
				do {
					let posts = self.postsToCheck
					sleep(60)
					if !self.running {
						return
					}
					
					guard !posts.isEmpty else {
						continue
					}
					
					//print("Checking \(posts.count) posts.")
					self.postsToCheck = self.postsToCheck.filter {!posts.contains($0)}
					for post in try apiClient.fetchQuestions(posts).items ?? [] {
						//don't report posts that are more than a day old
						let creation = (post.creation_date ?? Date()).timeIntervalSinceReferenceDate
						let activity = (post.last_activity_date ?? Date()).timeIntervalSinceReferenceDate
						
						if creation < (activity - 60 * 60 * 24) {
							continue
						}
						
						try self.checkAndReportPost(post)
					}
				} catch {
					handleError(error, "while checking active posts")
				}
			}
		}
	}
	
	private func attemptReconnect() {
		var done = false
		repeat {
			do {
				if wsRetries >= wsMaxRetries {
					fatalError(
						"Realtime questions websocket died; failed to reconnect!  Active posts will not be reported until a reboot. \(ping)"
					)
				}
				wsRetries += 1
				try start()
				done = true
			} catch {
				done = false
				sleep(5)
			}
		} while !done
	}
	
	func webSocketEnd(_ code: Int, reason: String, wasClean: Bool, error: Error?) {
		if let e = error {
			print("Websocket error:\n\(e)")
		}
		else {
			print("Websocket closed")
		}
		
		if running {
			print("Trying to reconnect...")
			attemptReconnect()
		}
	}
}
