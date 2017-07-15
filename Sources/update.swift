//
//  update.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE

func launchProcess(path: String, arguments: [String]) -> Process {
    return Process.launchedProcess(launchPath: path, arguments: arguments)
}

var isUpdating = false

func installUpdate() -> Bool {
    do {
        let compile = "./build.sh"
        let move = "mv ./.build/debug/FireAlarm .. && gunzip -c filter_static.sqlite.gz > ../filter_static.sqlite"
        let updateScript = "pushd .;" +
            "(cd update && " +
            compile + "&& " +
            move + " && " +
            "git log --format='oneline' -n 1 > ../version-new.txt && " +
            "cd .. && " +
            "rm -rf update) || " +
            "(popd; " +
        "touch update-failure)"
        
        if FileManager.default.fileExists(atPath: "update-failure") {
            try FileManager.default.removeItem(atPath: "update-failure")
        }
        
        try updateScript.write(toFile: "update.sh", atomically: true, encoding: .utf8)
        
        let process = launchProcess(path: "/bin/bash", arguments: ["update.sh"])
        process.waitUntilExit()
        
        
        return !FileManager.default.fileExists(atPath: "update-failure")
        
    } catch {
        handleError(error, "while updating")
    }
    
    return false
}

private enum DownloadFailure: Error {
    case noAutoupdate
    case downloadFailed
    case incorrectVersion(expected: String, downloaded: String)
}

var downloadedVersion: String = "<unknown>"
var availableVersion: String = "<unknown>"

func downloadUpdate(commit: String? = nil) throws {
    let script = "rm -rf update;" +
        "(git clone --single-branch -b \(updateBranch) \"git://github.com/SOBotics/FireAlarm.git\" update && " +
        "cd update" +
        (commit != nil ? "&& git checkout '\(commit!)'" : "") +
    ") || exit 1 "
    
    let process = launchProcess(path: "/bin/bash", arguments: ["-c", script])
    process.waitUntilExit()
    
    if process.terminationStatus != 0 {
        throw DownloadFailure.downloadFailed
    }
}

func getCurrentVersion() -> String {
    return (try? loadFile("version.txt").replacingOccurrences(of: "\n", with: "")) ?? "<unknown version>"
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


func prepareUpdate(to commit: String? = nil, listener: ChatListener, rooms: [ChatRoom]) -> Bool {
    if isUpdating {
        return true
    }
    
    isUpdating = true
    rooms.forEach { $0.postMessage("Updating...") }
    
    do {
        try downloadUpdate(commit: commit)
    } catch {
        handleError(error, "while downloading an update")
        isUpdating = false
        return false
    }
    
    if installUpdate() {
        rooms.forEach { $0.postMessage("Update complete; rebooting...") }
        listener.stop(.update)
    } else {
        isUpdating = false
        rooms.forEach { $0.postMessage("Update failed!") }
    }
    return true
}


func update(to commit: String? = nil, listener: ChatListener, rooms: [ChatRoom], force: Bool = false) -> Bool {
    if noUpdate {
        return false
    }
    
    if force {
        return prepareUpdate(to: commit, listener: listener, rooms: rooms)
    }
    
    
    let versionScript = "git ls-remote git://github.com/SOBotics/FireAlarm \(updateBranch) | cut -d '\t' -f1 > available_version.txt"
    
    
    do {
        try versionScript.write(toFile: "get_version.sh", atomically: true, encoding: .utf8)
        let process = launchProcess(path: "/bin/bash", arguments: ["get_version.sh"])
        process.waitUntilExit()
        
        let versionContents = try loadFile("available_version.txt").replacingOccurrences(of: "\n", with: " ")
        let components = versionContents.components(separatedBy: " ")
        availableVersion = components.first ?? ""
        
        if (commit != nil && commit != currentVersion) || (commit == nil && currentVersion != availableVersion),
            !(downloadedVersion == availableVersion && downloadedVersion != "<unknown>") {
            
            return prepareUpdate(to: commit, listener: listener, rooms: rooms)
        }
    }
    catch {
        handleError(error, "while checking for updates")
    }
    return false
}
