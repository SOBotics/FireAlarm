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

extension Post {
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

struct FilterResult {
	enum ResultType {
		case bayesianFilter(difference: Int)
		case customFilter(filter: Filter)
        case manuallyReported
	}
	
	let type: ResultType
	let header: String
	let details: String?
	
	init(type: ResultType, header: String, details: String? = nil) {
		self.type = type
		self.header = header
		self.details = details
	}
}

protocol Filter: class {
    init(reporter: Reporter)
    func check(_ post: Post, site: Site) throws -> FilterResult?
	func save() throws
}
