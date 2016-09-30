//
//  Post.swift
//  FireAlarm
//
//  Created by NobodyNada on 9/25/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

class Post {
	let id: Int
	let title: String
	let body: String
	let tags: [String]
	
	init(id: Int, title: String, body: String, tags: [String]) {
		self.id = id
		self.title = title
		self.body = body
		self.tags = tags
	}
}
