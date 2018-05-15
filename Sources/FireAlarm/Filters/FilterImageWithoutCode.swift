//
//  FilterImageWithoutCode.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 04/05/18.
//

import Foundation
import SwiftStack
import SwiftChatSE
import Dispatch

class FilterImageWithoutCode: Filter {
    required init(reporter: Reporter) {}
    
    func check(_ post: Post, site: Site) -> FilterResult? {
        //Filter weight; increase this is the filter is very accurate, decrease otherwise. Will get subtracted from Naive Bayes difference.
        let reasonWeight = 13
        
        if post.body!.contains("<img src") && !post.body!.contains("<code>") {
            let header = "Image without code"
            let details = header + " (weight \(reasonWeight))"
            return FilterResult (
                type: .customFilterWithWeight(filter: self, weight: reasonWeight),
                header: header,
                details: details
            )
        } else {
            return nil
        }
    }
    
    func save() throws {}
}
