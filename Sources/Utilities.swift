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
var reportHeader = "[ [FireAlarm-Swift](//stackapps.com/q/7183) ]"

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
	func notificationString(tags: [String], reasons: [FilterResult]) -> String {
		var users = [ChatUser]()
		for user in userDB {
			var shouldNotify = false
			
			if user.notified {
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
				
				for reason in reasons {
					guard case .customFilter(let filter) = reason.type else { continue }
					
					if filter is FilterBlacklistedUsernames && user.notificationReasons.contains("username") {
						shouldNotify = true
					}
					if filter is FilterMisleadingLinks && user.notificationReasons.contains("misleadingLink") {
						shouldNotify = true
					}
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
