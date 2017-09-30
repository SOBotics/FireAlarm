//
//  CommandGitStatus.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/30/17.
//

import Foundation
import SwiftChatSE

class CommandGitStatus: Command {
    override class func usage() -> [String] {
        return ["git status", "gitstatus"]
    }
    
    override func run() throws {
        let result = FireAlarm.run(command: "git status")
        if result.exitCode != 0 {
            if result.combinedOutput != nil {
                reply("`git status` returned exit code \(result.exitCode):")
            } else {
                reply("`git status` returned exit code \(result.exitCode).")
            }
        } else {
            if result.combinedOutput == nil {
                reply("The output of `git status` was not valid UTF-8.")
            }
        }
        if let output = result.combinedOutput {
            post("    " + output.components(separatedBy: .newlines).joined(separator: "\n    "))
        }
    }
}
