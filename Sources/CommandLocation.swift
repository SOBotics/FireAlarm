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
        do {
            let rawLocation = try loadFile ("location.txt")
			let location = String(rawLocation.characters.filter { !"\n".characters.contains($0) })
			reply ("I'm running on `\(location)`.")
        } catch {
            //handleError(error, "while loading location")
			reply("I don't know my location.")
        }
		
    }
}

