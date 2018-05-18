//
//  FilterNonEnglishPost.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 18/05/18.
//

import Foundation
import SwiftStack
import SwiftChatSE
import Dispatch

class FilterNonEnglishPost: Filter {
    required init(reporter: Reporter) {}
  
    //Take from https://stackoverflow.com/a/27880748/4688119
    func regexMatches(for regex: String, in text: String) -> [String] {
        do {
            let regex = try NSRegularExpression(pattern: regex)
            let results = regex.matches(in: text,
                                        range: NSRange(text.startIndex..., in: text))
            return results.map {
                String(text[Range($0.range, in: text)!])
            }
        } catch let error {
            print("invalid regex: \(error.localizedDescription)")
            return []
        }
    }
    
    func extractParagraphs(in text: String) -> String {
        //Removing html tags using https://stackoverflow.com/q/25983558/4688119; kinda hacky, but works.
        return regexMatches(for: "<p>.*?</p>", in: text).joined(separator: " ").replacingOccurrences(of: "<[^>]+>", with: "", options: .regularExpression, range: nil)
    }
    
    func check(_ post: Post, site: Site) -> FilterResult? {
        if #available(OSX 10.13, *) {
            if NSLinguisticTagger.dominantLanguage(for: extractParagraphs(in: post.body!)) != "en" {
                let header = "Non English Post"
                let details = header
                return FilterResult (
                    type: .customFilter(filter: self),
                    header: header,
                    details: details
                )
            } else {
                return nil
            }
        } else {
            return nil
        }
    }
    
    func save() throws {}
}
