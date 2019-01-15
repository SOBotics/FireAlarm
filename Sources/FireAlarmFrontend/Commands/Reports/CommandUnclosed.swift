//
//  CommandUnclosed.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 05/03/17.
//  Copyright © 2017 Ashish Ahuja. All right reserved
//

//TODO:
// 1. Instead of printing the reports in chat send them to Sam's server

import Foundation
import SwiftStack
import SwiftChatSE
import Dispatch

class CommandUnclosed: Command {
	override class func usage() -> [String] {
		return ["unclosed reports ..."]
	}
	
	override func run() throws {
		
		if (reportedPosts.isEmpty)
		{
			reply ("no reports!")
			return
		}
		
		if (arguments.count == 0) {
			reply ("Please specify the number of posts to be checked.")
			return
		}
		
		var totalToCheck: Int!
		let maxThreshold = 100
		var threshold: Int! = maxThreshold
		var maxCV: Int! = 4
		var minCV: Int! = 0
		
		if let total = Int(arguments[0]) {
			if (total > 40)
			{
				reply ("You cannot specify to check more than the last 40 reports!")
				return
			}
			totalToCheck = total
		} else {
			reply ("Please enter a valid number for the total reports to be checked.")
			return
		}
		
		if (arguments.count > 1) {
			for i in 1..<arguments.count {
				if (arguments[i].range(of: "t=") != nil || arguments[i].range(of: "threshold=") != nil) {
					if (arguments[i].range(of: "t=") != nil) {
						threshold = Int (arguments[i].replacingOccurrences(of: "t=", with: ""))
					} else if arguments[i].range(of: "threshold=") != nil {
						threshold = Int (arguments[i].replacingOccurrences(of: "threshold=", with: ""))
					}
					
					if (threshold > maxThreshold)
					{
						threshold = maxThreshold
					}
				}
				
				if (arguments [i].range(of: "cv=") != nil)
				{
					maxCV = Int (arguments[i].replacingOccurrences(of: "cv=", with: ""))
					minCV = maxCV
				} else if (arguments [i].range(of: "cv<") != nil) {
					maxCV = Int(arguments[i].replacingOccurrences(of: "cv<", with: "")) ?? 5 - 1
				} else if (arguments [i].range(of: "cv>") != nil){
					minCV = Int(arguments[i].replacingOccurrences(of: "cv>", with: "")) ?? -1 + 1
				} else if (arguments [i].range(of: "cv<=") != nil) {
					maxCV = Int(arguments[i].replacingOccurrences(of: "cv<=", with: ""))
				} else if (arguments[i].range(of: "cv>=") != nil) {
					minCV = Int(arguments [i].replacingOccurrences(of: "cv>=", with: ""))
				}
			}
		}
		
		var postsToCheck = [Int]()
		var postDifferences = [Int?]()
		
		if let minDate: Date = Calendar(identifier: .gregorian).date(byAdding: DateComponents(hour: -10), to: Date()) {
			let recentlyReportedPosts = reportedPosts.filter {
				$0.when > minDate
			}
			
			if (recentlyReportedPosts.count == 0)
			{
				reply ("There are no reports made by me recently, thus I have nothing to check!")
				return
			}
			
			if (totalToCheck > recentlyReportedPosts.count)
			{
				totalToCheck = recentlyReportedPosts.count
			}
			
			var i = 0
			while postsToCheck.count < recentlyReportedPosts.count {
				postsToCheck.append(recentlyReportedPosts[i].id)
				postDifferences.append(recentlyReportedPosts[i].difference)
				i = i + 1
			}
		}
		else {
			message.room.postMessage("Failed to calculate minimum report date!")
		}
		
		var messageClosed = ""
		var totalPosts = 0
		var i = 0
		
		//Now fetch the posts from the API
		for post in try apiClient.fetchQuestions(postsToCheck).items ?? [] {
			if totalPosts < totalToCheck &&
				postDifferences [i] ?? 0 < threshold && post.closed_reason == nil &&
				post.close_vote_count ?? 0 >= minCV &&
				post.close_vote_count ?? 0 <= maxCV
			{
				messageClosed = messageClosed + "\nCV:\(post.close_vote_count ?? 0)   \(post.link?.absoluteString ?? "https://example.com")"
				totalPosts = totalPosts + 1
			}
			i = i + 1
		}
		
		message.room.postMessage (messageClosed)
	}
}
