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
}

class FilterNaiveBayes: Filter {
    let words: [String:Word]
    let initialProbability: Double
    
    init () {
        print ("Loading Naive Bayes filter...")
        
        let data = try! Data(contentsOf: saveDirURL.appendingPathComponent("filter.json"))
        let db = try! JSONSerialization.jsonObject(with: data, options: []) as! [String:Any]
        initialProbability = db["initialProbability"] as! Double
        var words = [String:Word]()
        for (word, probabilites) in db["wordProbabilities"] as! [String:[Double]] {
            words[word] = Word(word, probabilites.first!, probabilites.last!)
        }
        self.words = words
    }
    
    func check(_ post: Question) -> FilterResult? {
        var trueProbability = Double(0.263)
        var falseProbability = Double(1 - trueProbability)
        var postWords = [String]()
        var checkedWords = [String]()
        
        guard let body = post.body else {
            print("No body for \(post.id.map { String($0) } ?? "<no ID>")")
            return nil
        }
        
        var currentWord: String = ""
        let set = CharacterSet.alphanumerics.inverted
        for character in body.lowercased().characters {
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
            guard let word = words[postWord] else {
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
	
	func save() throws {
		
	}
}
