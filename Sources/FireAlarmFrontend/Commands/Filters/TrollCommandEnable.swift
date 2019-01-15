//
//  TrollCommandEnable.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 3/23/18.
//

import Foundation
import SwiftChatSE
import SwiftStack

class TrollCommandEnable: Command {
    override class func usage() -> [String] {
        return ["troll ..."]
    }
    
    override func run() throws {
        if arguments.isEmpty {
            reporter.trollSites = .all
            reply("Watching a troll on all sites.")
            return
        }
        
        var allSites = [SwiftStack.Site]()
        allSites.reserveCapacity(200)
        
        var hasMore = false
        var page = 1
        let pageSize = 100
        repeat {
            let response = try apiClient.fetchSites(parameters: ["per_page": String(pageSize), "page": String(page)])
            guard let sites = response.items else {
                reply("Failed to fetch sites!")
                return
            }
            
            allSites.append(contentsOf: sites)
            
            hasMore = response.has_more ?? false
            page += 1
        } while hasMore
        
        var sites = [Site]()
        sites.reserveCapacity(arguments.count)
        
        for siteName in arguments {
            guard let index = allSites.index(where: { $0.api_site_parameter == siteName || $0.site_url?.host == siteName }) else {
                reply("Unknown site \(siteName).")
                return
            }
            
            let apiSite = allSites[index]
            
            guard let apiSiteParameter = apiSite.api_site_parameter, let domain = apiSite.site_url?.host else {
                reply("Failed to retrieve site information!")
                return
            }
            
            sites.append(Site(id: -1, apiSiteParameter: apiSiteParameter, domain: domain, initialProbability: 0))
        }
        
        sites.forEach { reporter.trollSites.add($0) }
        switch reporter.trollSites {
        case .all: reply("Watching a troll on all sites.")
        case .sites(let sites): reply("Watching a troll on " + formatArray(sites.map { $0.domain }, conjunction: "and") + ".")
        }
    }
}
