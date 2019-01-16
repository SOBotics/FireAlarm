//
//  Filter.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/24/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE
import SwiftStack
import Dispatch

public extension Post {
	var id: Int? {
		if let q = self as? Question, let id = q.question_id {
			return id
		} else if let a = self as? Answer, let id = a.answer_id {
			return id
		} else {
			return post_id
		}
	}
}

public struct FilterResult {
	public enum ResultType {
		case bayesianFilter(difference: Int)
		case customFilter(filter: Filter)
        case customFilterWithWeight(filter: Filter, weight: Int)
        case manuallyReported
	}
	
	public let type: ResultType
	public let header: String
	public let details: String?
	
	public init(type: ResultType, header: String, details: String? = nil) {
		self.type = type
		self.header = header
		self.details = details
	}
}

public protocol Filter: class {
    func check(post: Post, site: String) throws -> FilterResult?
	func save() throws
}
