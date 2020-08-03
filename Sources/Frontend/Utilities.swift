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
import Dispatch
import FireAlarmCore

let saveDirURL = URL(fileURLWithPath: FileManager.default.currentDirectoryPath)

var startTime = Date()

var botName = "FireAlarm"
var githubLink = "//github.com/SOBotics/FireAlarm/tree/swift"
var stackAppsLink = "//stackapps.com/q/7183"
var reportHeader = "[ [\(botName)](//stackapps.com/q/7183) ]"

var currentVersion = "<unknown version>"
var shortVersion = "<unknown version>"
var versionLink = githubLink

var location = "<unknown location>"
var user = "<unknown user>"
var device = "<unknown device>"

var originalWorkingDirectory: String!


let updateBranch = "master"

var development = false
var noUpdate = false

extension ChatUser {
    enum NotificationReason {
        case misleadingLinks
        case blacklist(BlacklistManager.BlacklistType)
        case tag(String)
        case all
        
        var asString: String {
            switch self {
            case .misleadingLinks: return "misleading_links"
            case .blacklist(let list): return "blacklist:\(list.rawValue)"
            case .tag(let tag): return "tag:\(tag)"
            case .all: return "all"
            }
        }
        init?(string: String) {
            if string == "misleading_links" { self = .misleadingLinks }
            else if string == "all" { self = .all }
            else {
                let components = string.components(separatedBy: ":")
                guard components.count >= 2 else { return nil }
                let type = components.first!
                let value = components.dropFirst().joined(separator: ":")
                switch type {
                case "blacklist":
                    if let result = BlacklistManager.BlacklistType(rawValue: value).map(NotificationReason.blacklist) {
                        self = result
                    } else {
                        return nil
                    }
                case "tag":
                    self = .tag(value)
                default:
                    return nil
                }
            }
        }
        
        var description: String {
            switch self {
            case .misleadingLinks: return "misleading link"
            case .blacklist(let list): return "blacklisted \(list.rawValue)"
            case .tag(let tag): return "\(tag)"
            case .all: return "all reports"
            }
        }
    }
    var notificationReasons: [NotificationReason] {
        get {
            return (info["notificationReasons"] as? [String])?.compactMap { NotificationReason(string: $0) } ?? []
        } set {
            info["notificationReasons"] = newValue.map { $0.asString }
        }
    }
}

extension ChatRoom {
    func notificationString(tags: [String], reasons: [FilterResult]) -> String {
        var users = [ChatUser]()
        for user in userDB {
            let shouldNotify = user.notificationReasons.contains { notificationReason in
                switch notificationReason {
                case .all: return true
                case .blacklist(let type):
                    return reasons.contains {
                        if case .customFilter(let filter) = $0.type {
                            return (filter as? BlacklistFilter).map { $0.blacklistType == type } ?? false
                        } else {
                            return false
                        }
                    }
                case .misleadingLinks:
                    return reasons.contains {
                        if case .customFilter(let filter) = $0.type {
                            return filter is FilterMisleadingLinks
                        } else {
                            return false
                        }
                    }
                case .tag(let tag):
                    return tags.contains(tag)
                }
            }
            
            if shouldNotify {
                users.append(user)
            }
        }
        
        return users.map { "@" + $0.name.replacingOccurrences(of: " ", with: "") }.joined(separator: " ")
    }
    
    
    private func stringify(thresholds: [Int:Int]) -> [String:Int] {
        var result = [String:Int]()
        for (key, value) in thresholds {
            result[String(key)] = value
        }
        return result
    }
    
    private func destringify(thresholds: [String:Int]) -> [Int:Int] {
        var result = [Int:Int]()
        for (key, value) in thresholds {
            if let siteID = Int(key) {
                result[siteID] = value
            }
        }
        return result
    }
    
    ///A dictionary mapping sites to thresholds.
    var thresholds: [Int:Int] {
        get {
            return (info["thresholds"] as? [String:Int]).map(destringify) ?? (info["threshold"] as? Int).map { [2:$0] } ?? [:]
        } set {
            info["thresholds"] = stringify(thresholds: newValue)
        }
    }
    
    convenience init(client: Client, host: Host, roomID: Int, thresholds: [Int:Int]) {
        self.init(client: client, host: host, roomID: roomID)
        self.thresholds = thresholds
    }
}

extension ChatUser.Privileges {
    static let filter = ChatUser.Privileges(rawValue: 1 << 1)
}

func addPrivileges() {
    ChatUser.Privileges.add(name: "Filter", for: .filter)
}

func run(command: String, printOutput: Bool = true) -> (exitCode: Int, stdout: String?, stderr: String?, combinedOutput: String?) {
    let process = Process()
    process.launchPath = "/usr/bin/env"
    process.arguments = ["bash", "-c", command]
    
    let stdoutPipe = Pipe()
    let stderrPipe = Pipe()
    process.standardOutput = stdoutPipe
    process.standardError = stderrPipe
    
    var stdout = Data()
    var stderr = Data()
    var combined = Data()
    
    let queue = DispatchQueue(label: "org.SOBotics.firealarm.commandOutputQueue")
    let stdoutSource = DispatchSource.makeReadSource(fileDescriptor: stdoutPipe.fileHandleForReading.fileDescriptor)
    let stderrSource = DispatchSource.makeReadSource(fileDescriptor: stderrPipe.fileHandleForReading.fileDescriptor)
    
    stdoutSource.setEventHandler {
        queue.sync {
            let data = stdoutPipe.fileHandleForReading.availableData
            stdout += data
            combined += data
            if printOutput {
                FileHandle.standardError.write(data)
            }
        }
    }
    stderrSource.setEventHandler {
        queue.sync {
            let data = stderrPipe.fileHandleForReading.availableData
            stderr += data
            combined += data
            if printOutput {
                FileHandle.standardOutput.write(data)
            }
        }
    }
    
    stdoutSource.resume()
    stderrSource.resume()
    
    process.launch()
    process.waitUntilExit()
    
    queue.sync {
        stdoutSource.cancel()
        stderrSource.cancel()
    }
    queue.sync {
        stdoutPipe.fileHandleForReading.closeFile()
        stderrPipe.fileHandleForReading.closeFile()
    }
    
    let stdoutString = String(data: stdout, encoding: .utf8)
    let stderrString = String(data: stderr, encoding: .utf8)
    let combinedString = String(data: combined, encoding: .utf8)
    return (exitCode: Int(process.terminationStatus), stdout: stdoutString, stderr: stderrString, combinedOutput: combinedString)
}
