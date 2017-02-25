//
//  CommandLocation.swift
//  FireAlarm
//
//  Created by Ashish Ahuja on 25/02/17.
//  Copyright © 2017 Ashish Ahuja. All right reserved
//

import Foundation
import SwiftChatSE

class CommandLocation: Command {
    override class func usage() -> [String] {
        return ["location"]
    }
    
    override func run() throws {
        //Get the location
        var rawLocation = ""
        var location = "<unknown>"
        do {
            rawLocation = try loadFile ("location.txt")
            location = String(rawLocation.characters.filter { !"\n".characters.contains($0) })
        } catch {
            //handleError(error, "while loading location")
            location = "<unknown>"
        }
        
        reply ("I'm running at `\(location)`.")
    }
}

