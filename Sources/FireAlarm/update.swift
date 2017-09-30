//
//  update.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

var isUpdating = false

func getCurrentVersion() -> String {
    let versionResults = run(command: "git rev-parse HEAD")
    return (versionResults.exitCode != 0 ? versionResults.stdout : nil)?.replacingOccurrences(of: "\n", with: "") ?? "<unknown version>"
}

public func getShortVersion(_ version: String) -> String {
    if version == "<unknown version>" {
        return "<unknown>"
    }
    return version.characters.count > 7 ?
        String(version.characters[version.characters.startIndex..<version.characters.index(version.characters.startIndex, offsetBy: 7)]) :
    "<unknown version>"
}

public func getVersionLink(_ version: String) -> String {
    if version == "<unknown version>" {
        return githubLink
    } else {
        return "//github.com/SOBotics/FireAlarm/commit/\(version)"
    }
}


func update(to commit: String?, listener: ChatListener, rooms: [ChatRoom], force: Bool = false) -> Bool {
    if noUpdate || isUpdating {
        return false
    }
    isUpdating = true
    defer { isUpdating = false }
    
    let pullResult = run(command: commit != nil ? "git fetch && git merge \(commit!)" : "git pull")
    if pullResult.exitCode != 0 {
        if let output = pullResult.combinedOutput, !output.isEmpty {
            let message = "    " + output.components(separatedBy: .newlines).joined(separator: "\n    ")
            rooms.forEach {
                $0.postMessage("\(reportHeader) Update failed:")
                $0.postMessage(message)
            }
        } else {
            rooms.forEach { $0.postMessage("\(reportHeader) Update failed!") }
        }
    }
    
    if !force && pullResult.stdout == "Already up-to-date.\n" { return false }
    
    rooms.forEach { $0.postMessage("\(reportHeader) Updating...") }
    
    let buildResult = run(command: "swift build -c release")
    if buildResult.exitCode != 0 {
        if let output = buildResult.combinedOutput, !output.isEmpty {
            let message = "    " + output.components(separatedBy: .newlines).joined(separator: "\n    ")
            rooms.forEach {
                $0.postMessage("\(reportHeader) Update failed:")
                $0.postMessage(message)
            }
        } else {
            rooms.forEach { $0.postMessage("\(reportHeader) Update failed!") }
        }
    } else {
        rooms.forEach { $0.postMessage("\(reportHeader) Update complete; rebooting...") }
        listener.stop(.reboot)
    }
    
    return true
}
