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

var reportedPosts = [(id: Int, when: Date, difference: Int)]()

class Reports {
    enum ReportsLoadingError: Error {
        case ReportsNotArrayOfDictionaries
        case InvalidReport(report: [String:Any])
    }
    
    init () {
        let reportsURL = saveDirURL.appendingPathComponent("reports.json")
        let usernameURL = saveDirURL.appendingPathComponent("blacklisted_users.json")
        do {
            let reportData = try Data(contentsOf: reportsURL)
            guard let reports = try JSONSerialization.jsonObject(with: reportData, options: []) as? [[String:Any]] else {
                throw ReportsLoadingError.ReportsNotArrayOfDictionaries
            }
            
            reportedPosts = try reports.map {
                guard let id = $0["id"] as? Int, let when = $0["t"] as? Int else {
                    throw ReportsLoadingError.InvalidReport(report: $0)
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
    }
    
    func saveReports() throws {
        let data = try JSONSerialization.data(
            withJSONObject: reportedPosts.map {
                ["id":$0.id,"t":Int($0.when.timeIntervalSince1970),"d":$0.difference]
            }
        )
        
        try data.write(to: saveDirURL.appendingPathComponent("reports.json"))
    }
}
