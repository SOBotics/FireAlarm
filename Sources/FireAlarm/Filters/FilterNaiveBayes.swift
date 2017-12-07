//  FilterNaiveBayes.swift
//  FireAlarm
//
//  Created by AshishAhuja on 23/04/17.
//  Copyright © 2017 Ashish Ahuja (Fortunate-MAN). All rights reserved.
//

import Foundation
import SwiftChatSE
import Dispatch
import SwiftStack

class Word {
    let text: String
    let trueProbability: Double
    let falseProbability: Double
    
    init(_ text: String, _ pTrue: Double, _ pFalse: Double) {
        self.text = text
        trueProbability = pTrue
        falseProbability = pFalse
    }
    
    convenience init(row: Row) {
        self.init(
            row.column(named: "word")!,
            row.column(named: "true")!,
            row.column(named: "false")!
        )
    }
}

class FilterNaiveBayes: Filter {
    required init(reporter: Reporter) {
        print ("Loading Naive Bayes filter...")
    }
    
    func check(_ post: Question, site: Site) throws -> FilterResult? {
        var trueProbability = Double(site.initialProbability)
        var falseProbability = Double(1 - trueProbability)
        var postWords = [String]()
        var checkedWords = [String]()
        
        guard let body = post.body else {
            print("No body for \(post.id.map { String($0) } ?? "<no ID>")")
            return nil
        }
        
        var currentWord: String = ""
        let set = CharacterSet.alphanumerics.inverted
        for character in body.lowercased() {
            if !set.contains(String(character).unicodeScalars.first!) {
                currentWord.append(character)
            }
            else if !currentWord.isEmpty {
                postWords.append(currentWord)
                currentWord = ""
            }
        }
        
        if !currentWord.isEmpty {
            postWords.append(currentWord)
        }
        
        for postWord in postWords {
            if postWord.isEmpty {
                continue
            }
            guard let word = try reporter.staticDB.run(
                "SELECT * FROM words " +
                "WHERE site = ? " +
                "AND word = ?;",
                site.id, postWord
                ).first.map(Word.init) else {
                    
                continue
            }
            checkedWords.append(postWord)
            
            let pTrue = word.trueProbability
            let pFalse = word.falseProbability
            
            
            let newTrue = trueProbability * Double(pTrue)
            let newFalse = falseProbability * Double(pFalse)
            if newTrue != 0.0 && newFalse != 0.0 {
                trueProbability = newTrue
                falseProbability = newFalse
            }
        }
        
        let difference = -log10(falseProbability - trueProbability)
        
		if difference.isNormal {
			return FilterResult(
				type: .bayesianFilter(difference: Int(difference)),
				header: "Potentially bad question",
				details: "Naive Bayes score \(Int(difference))"
			)
		}
		
		return nil
    }
	
    func save() throws {}
}
