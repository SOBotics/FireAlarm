//
//  BlacklistManager.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 10/18/17.
//

import Foundation

class Blacklist: Codable {
    var items: [String]
    
    func encode(to encoder: Encoder) throws {
        try items.encode(to: encoder)
    }
    
    required init(from decoder: Decoder) throws {
        items = try [String](from: decoder)
    }
    
    required init() {
        items = []
    }
    
    func items(catching string: String) -> [String] {
        return items.filter { string.range(of: $0, options: [.regularExpression, .caseInsensitive]) != nil }
    }
    
    ///Adds an item to the blacklist.
    ///- returns: `true` if the item was added; `false` if it was already present.
    @discardableResult func add(item: String) -> Bool {
        if items.contains(item) { return false }
        items.append(item)
        return true
    }
    
    ///Removes an item from the blacklist.
    ///- returns: `true` if the item was removed; `false` if it was not present.
    @discardableResult func remove(item: String) -> Bool {
        guard let index = items.index(of: item) else { return false }
        items.remove(at: index)
        return true
    }
}

class BlacklistManager {
    private var blacklists: [String:Blacklist]
    
    init(blacklists: [String:Blacklist] = [:]) {
        self.blacklists = blacklists
    }
    
    convenience init(json: Data) throws {
        self.init(blacklists: try JSONDecoder().decode([String:Blacklist].self, from: json))
    }
    
    convenience init(url: URL) throws {
        try self.init(json: Data(contentsOf: url))
    }
    
    func save(url: URL) throws {
        try JSONEncoder().encode(blacklists).write(to: url)
    }
    
    enum BlacklistType: String {
        case username
        case keyword
        case tag
        
        init?(name: String) {
            if let result = BlacklistType.init(rawValue: name) {
                self = result
            } else if let result = BlacklistType.init(rawValue: name + "s") {
                self = result
            } else if name.last == "s", let result = BlacklistType.init(rawValue: String(name.dropLast())) {
                self = result
            } else {
                return nil
            }
        }
    }
    
    func blacklist(ofType type: BlacklistType) -> Blacklist {
        if let blacklist = blacklists[type.rawValue] {
            return blacklist
        } else {
            let blacklist = Blacklist()
            self.blacklists[type.rawValue] = blacklist
            return blacklist
        }
    }
}
