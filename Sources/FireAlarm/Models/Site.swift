//
//  Site.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 10/7/17.
//

import Foundation
import SwiftChatSE

struct Site: Codable {
    var id: Int
    var apiSiteParameter: String
    var domain: String
    var initialProbability: Float
    
    static func from(row: Row) throws -> Site {
        return try RowDecoder().decode(Site.self, from: row)
    }
    
    static func with(id: Int, db: DatabaseConnection) throws -> Site? {
        return try db.run("SELECT * FROM sites WHERE id = ?;", id).first.map(Site.from)
    }
}
