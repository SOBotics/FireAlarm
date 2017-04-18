//
//  main.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/27/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import SwiftChatSE
#if os(Linux)
	import Glibc
#endif

do {
	try main()
} catch {
	handleError(error, "while starting up")
	abort()
}

dispatchMain()
