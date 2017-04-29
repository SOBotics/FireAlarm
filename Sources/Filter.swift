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

class Filter {
	
}
