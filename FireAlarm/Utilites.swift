//
//  Utilites.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 11/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import Dispatch



enum FileLoadingError: Error {
	case notUFT8
}

func loadFile(_ path: String) throws -> String {
	let data = try Data(contentsOf: URL(fileURLWithPath: path))
	guard let str = String(data: data, encoding: .utf8) else {
		throw FileLoadingError.notUFT8
	}
	
	return str
}

func formatArray<T>(_ array: [T], conjunction: String) -> String {
	var string = ""
	if array.count == 1 {
		string = "\(array.first!)"
	}
	else {
		for (index, item) in array.enumerated() {
			if index == array.count - 1 {
				string.append("\(conjunction) \(item)")
			}
			else {
				string.append("\(item)\(array.count == 2 ? "" : ",") ")
			}
		}
	}
	return string
}

func pluralize(_ n: Int, _ singular: String, _ plural: String? = nil) -> String {
	let resultPlural: String
	if let p = plural {
		resultPlural = p
	} else {
		resultPlural = singular + "s"
	}
	
	if n == 1 {
		return singular
	} else {
		return resultPlural
	}
}


#if os(macOS)
	func clearCookies(_ storage: HTTPCookieStorage) {
		if let cookies = storage.cookies {
			for cookie in cookies {
				storage.deleteCookie(cookie)
			}
		}
	}
#endif

func makeTable(_ heading: [String], contents: [String]...) -> String {
	if heading.count != contents.count {
		fatalError("heading and contents have different counts")
	}
	let cols = heading.count
	
	var alignedHeading = [String]()
	var alignedContents = [[String]]()
	
	var maxLength = [Int]()
	
	var rows = 0
	var tableWidth = 0
	
	for col in 0..<cols {
		maxLength.append(heading[col].characters.count)
		for row in contents[col] {
			maxLength[col] = max(row.characters.count, maxLength[col])
		}
		rows = max(contents[col].count, rows)
		alignedHeading.append(heading[col].padding(toLength: maxLength[col], withPad: " ", startingAt: 0))
		alignedContents.append(contents[col].map {
			$0.padding(toLength: maxLength[col], withPad: " ", startingAt: 0)
			}
		)
		tableWidth += maxLength[col]
	}
	tableWidth += (cols - 1) * 3
	
	let head = alignedHeading.joined(separator: " | ")
	let divider = String([Character](repeating: "-", count: tableWidth))
	var table = [String]()
	
	for row in 0..<rows {
		var columns = [String]()
		for col in 0..<cols {
			columns.append(
				alignedContents[col].count > row ?
					alignedContents[col][row] : String([Character](repeating: " ", count: maxLength[col])))
		}
		table.append(columns.joined(separator: " | "))
	}
	
	return "    " + [head,divider,table.joined(separator: "\n    ")].joined(separator: "\n    ")
}



func postIDFromURL(_ url: URL, isUser: Bool = false) -> Int? {
	if url.host != "stackoverflow.com" && url.host != "www.stackoverflow.com" {
		return nil
	}
	
	let componentIndex: Int
	let component: String
	if url.pathComponents.first == "/" {
		if url.pathComponents.count < 3 {
			return nil
		}
		componentIndex = 1
	}
	else {
		if url.pathComponents.count < 2 {
			return nil
		}
		componentIndex = 0
	}
	component = url.pathComponents[componentIndex]
	
	if (isUser && (component == "u" || component == "users")) ||
		(!isUser && (component == "q" || component == "questions" ||
			component == "a" || component == "answers" ||
			component == "p" || component == "posts")) {
		return Int(url.pathComponents[componentIndex + 1])
	}
	return nil
}

public var botName = "FireAlarm-Swift"
public var githubLink = "//github.com/NobodyNada/FireAlarm/tree/swift"
