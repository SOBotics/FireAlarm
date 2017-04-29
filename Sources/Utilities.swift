//
//  Utilities.swift
//  FireAlarm
//
//  Created by NobodyNada on 4/18/17.
//
//

import Foundation
import SwiftStack
import SwiftChatSE

let saveDirURL = URL(fileURLWithPath: NSHomeDirectory()).appendingPathComponent(".firealarm", isDirectory: true)

var startTime = Date()

var botName = "FireAlarm-Swift"
var githubLink = "//github.com/SOBotics/FireAlarm/tree/swift"
var stackAppsLink = "//stackapps.com/q/7183"

var currentVersion = "<unknown version>"
var shortVersion = "<unknown version>"
var versionLink = githubLink

var location = "<unknown location>"
var user = "<unknown user>"
var device = "<unknown device>"

var originalWorkingDirectory: String!

var development = false
var noUpdate = false

extension ChatUser {
	var notified: Bool {
		get {
			return ((info["notified"] as? Int) ?? 0) == 1 ? true : false
		} set {
			info["notified"] = (newValue ? 1 : 0)
		}
	}
	var notificationTags: [String] {
		get {
			return (info["notificationTags"] as? [String]) ?? []
		} set {
			info["notificationTags"] = newValue
		}
	}
	var notificationReasons: [String] {
		get {
			return (info["notificationReasons"] as? [String]) ?? []
		} set {
			info["notificationReasons"] = newValue
		}
	}
}

extension ChatRoom {
	
	
	func notificationString(tags: [String], reason: PostClassifier.ReportReason) -> String {
		var users = [ChatUser]()
		for user in userDB {
			var shouldNotify = false
			
			if user.notified {
				switch reason {
				case .bayesianFilter:
					if !user.notificationTags.isEmpty {
						for tag in tags {
							if user.notificationTags.contains(tag) {
								shouldNotify = true
							}
						}
					}
					else {
						shouldNotify = true
					}
					
				case .blacklistedUsername:
					if (user.notificationReasons.isEmpty && user.notificationTags.isEmpty)
						|| user.notificationReasons.contains("username") {
						
						shouldNotify = true
					}
					
				case .misleadingLink:
					if (user.notificationReasons.isEmpty && user.notificationTags.isEmpty)
						|| user.notificationReasons.contains("misleadingLink") {
						
						shouldNotify = true
					}
					
				case .manuallyReported:
					break
				}
			}
			
			if shouldNotify {
				users.append(user)
			}
		}
		
		return users.map { "@" + $0.name.replacingOccurrences(of: " ", with: "") }.joined(separator: " ")
	}
	
	var threshold: Int {
		get {
			return (info["threshold"] as? Int) ?? 45
		} set {
			info["threshold"] = newValue
		}
	}
}

extension ChatUser.Privileges {
	static let filter = ChatUser.Privileges(rawValue: 1 << 1)
}

func addPrivileges() {
	ChatUser.Privileges.add(name: "Filter", for: .filter)
}
