//
//  Site.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 10/7/17.
//

import Foundation
import SwiftChatSE

struct Site: Codable, Hashable {
    var id: Int
    var apiSiteParameter: String
    var domain: String
    var initialProbability: Float
    
    static func from(row: Row) throws -> Site {
        return try RowDecoder().decode(Site.self, from: row)
    }
    
    ///Helper function used by methods such as with(id:db:).
    ///- warning: The `name` parameter is vulnerable to SQL injection; make sure it's valid!
    static private func with<T: DatabaseType>(parameter: T, name: String, db: DatabaseConnection) throws -> Site? {
        return try db.run("SELECT * FROM sites WHERE \(name) = ?;", parameter).first.map(Site.from)
    }
    
    ///Returns the Site with the specified ID.
    static func with(id: Int, db: DatabaseConnection) throws -> Site? {
        return try with(parameter: id, name: "id", db: db)
    }
    
    ///Returns the Site with the specified domain.
    static func with(domain: String, db: DatabaseConnection) throws -> Site? {
        return try with(parameter: domain, name: "domain", db: db)
    }
    
    ///Returns the Site with the specified apiSiteParameter.
    static func with(apiSiteParameter: String, db: DatabaseConnection) throws -> Site? {
        return try with(parameter: apiSiteParameter, name: "apiSiteParameter", db: db)
    }
    
    var hashValue: Int { return apiSiteParameter.hashValue }
    static func ==(lhs: Site, rhs: Site) -> Bool { return lhs.apiSiteParameter == rhs.apiSiteParameter }
}
