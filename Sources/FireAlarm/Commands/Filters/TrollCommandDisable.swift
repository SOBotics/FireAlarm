//
//  TrollCommandDisable.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 3/23/18.
//

import Foundation
import SwiftChatSE

class TrollCommandDisable: Command {
    override class func usage() -> [String] {
        return ["untroll ..."]
    }
    
    override func run() throws {
        if case .sites(let sites) = reporter.trollSites, !arguments.isEmpty {
            let filtered = sites.filter { !(arguments.contains($0.domain) || arguments.contains($0.apiSiteParameter)) }
            reporter.trollSites = .sites(filtered)
            reply("Only " + formatArray(filtered.map { "`\($0.domain)`" }, conjunction: "and") + " will be monitored.")
            return
        } else if !arguments.isEmpty {
            reply("All sites are monitored; run `@FireAlarm troll <sites>` to monitor only a select group of sites.")
            return
        }
        
        //Arguments is empty; disable all sites
        reporter.trollSites = .sites([])
        reply("Stopped monitoring on all sites.")
    }
}
