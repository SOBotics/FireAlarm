//
//  FilterCodeWithoutExplanation.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 16/05/18.
//

import Foundation
import SwiftStack
import SwiftChatSE
import Dispatch
import FireAlarmCore

//Detects post where code exists but no explanation, i.e, code dump questions.
class FilterCodeWithoutExplanation: Filter {
    init() {}
    
    func check(post: Post, site: String) -> FilterResult? {
        if post.body!.contains("<code>") && !post.body!.contains("<p>") {
            let header = "Code without explanation"
            let details = header
            return FilterResult (
                type: .customFilter(filter: self),
                header: header,
                details: details
            )
        } else {
            return nil
        }
    }
    
    func save() throws {}
}
