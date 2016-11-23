//
//  ErrorHandler.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 11/22/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

var errorRoom: ChatRoom?

func errorAsNSError(_ error: Error) -> NSError? {
	#if os(Linux)
		//this is the only way I could find to check if an arbitrary Error is an NSError that doesn't crash on Linux
		//it produces a warning "'is' test is always true"
		return error is AnyObject ? unsafeBitCast(error, to: NSError.self) : nil
	#else
		return type(of: error) == NSError.self ? error as NSError : nil
	#endif
}

func formatNSError(_ e: NSError) -> String {
	return "\(e.domain) code \(e.code) \(e.userInfo)"
}

func handleError(_ error: Error, _ context: String? = nil) {
	let contextStr: String
	let errorType: String
	let errorDetails: String
	
	#if os(Linux)
		if let e = errorAsNSError(error) {
			errorType = "NSError"
			errorDetails = formatNSError(e)
		} else {
			errorType = String(reflecting: type(of: error))
			errorDetails = String(describing: error)
		}
	#else
		errorType = String(reflecting: type(of: error))
		errorDetails = String(describing: error)
	#endif
	
	if context != nil {
		contextStr = " \(context!)"
	}
	else {
		contextStr = ""
	}
	
	let message1 = "    An error (\(errorType)) occured\(contextStr) (cc @NobodyNada):"
	
	if let room = errorRoom {
		room.postMessage(message1 + "\n    " + errorDetails.replacingOccurrences(of: "\n", with: "\n    "))
	}
	else {
		print("\(message1)\n\(errorDetails)")
		exit(1)
	}
}
