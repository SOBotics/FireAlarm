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

let saveDirURL = URL(fileURLWithPath: FileManager.default.currentDirectoryPath)

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


let updateBranch = "master"

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
    
    stdoutPipe.fileHandleForReading.readabilityHandler = { handle in
        let data = handle.availableData
        queue.sync {
            stdout += data
            combined += data
            if printOutput {
                FileHandle.standardOutput.write(data)
            }
        }
    }
    stderrPipe.fileHandleForReading.readabilityHandler = { handle in
        let data = handle.availableData
        queue.sync {
            stderr += data
            combined += data
            if printOutput {
                FileHandle.standardError.write(data)
            }
        }
    }
    
    process.launch()
    process.waitUntilExit()
    
    queue.sync {}
    stdoutPipe.fileHandleForReading.closeFile()
    stderrPipe.fileHandleForReading.closeFile()
    
    let stdoutString = String(data: stdout, encoding: .utf8)
    let stderrString = String(data: stderr, encoding: .utf8)
    let combinedString = String(data: combined, encoding: .utf8)
    return (exitCode: Int(process.terminationStatus), stdout: stdoutString, stderr: stderrString, combinedOutput: combinedString)
}
