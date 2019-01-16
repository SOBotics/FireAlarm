//
//  PostScanner.swift
//  FireAlarmCore
//
//  Created by NobodyNada on 9/22/18.
//

import Foundation
import SwiftStack

open class PostScanner {
    open var filters: [Filter]
    
    public init(filters: [Filter]) {
        self.filters = filters
    }
    
    open func scan(post: Post, site: String) throws -> [FilterResult] {
        return try filters.compactMap { try $0.check(post: post, site: site) }
    }
}
