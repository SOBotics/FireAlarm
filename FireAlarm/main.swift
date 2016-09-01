//
//  main.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/27/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

func clearCookies() {
    let storage = NSHTTPCookieStorage.sharedHTTPCookieStorage()
    if let cookies = storage.cookies {
        for cookie in cookies {
            storage.deleteCookie(cookie)
        }
    }
}



func makeTable(heading: [String], contents: [String]...) -> String {
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
        alignedHeading.append(heading[col].stringByPaddingToLength(maxLength[col], withString: " ", startingAtIndex: 0))
        alignedContents.append(contents[col].map {
            $0.stringByPaddingToLength(maxLength[col], withString: " ", startingAtIndex: 0)
            }
        )
        tableWidth += maxLength[col]
    }
    tableWidth += (cols - 1) * 3
    
    let head = alignedHeading.joinWithSeparator(" | ")
    let divider = String([Character](count: tableWidth, repeatedValue: "-"))
    var table = [String]()
    
    for row in 0..<rows {
        var columns = [String]()
        for col in 0..<cols {
            columns.append(
                alignedContents[col].count > row ?
                    alignedContents[col][row] : String([Character](count:maxLength[col], repeatedValue: " ")))
        }
        table.append(columns.joinWithSeparator(" | "))
    }
    
    return "    " + [head,divider,table.joinWithSeparator("\n    ")].joinWithSeparator("\n    ")
}



private var errorRoom: ChatRoom?
private enum BackgroundTask {
    case HandleInput(input: String)
    case ShutDown(reboot: Bool)
}

private var backgroundTasks = [BackgroundTask]()
private let backgroundSemaphore = dispatch_semaphore_create(0)



func main() throws {
    print("FireAlarm starting...")
    
    //Save the working directory so we don't break rebooting if we chdir.
    let originalWorkingDirectory = NSFileManager.defaultManager().currentDirectoryPath
    
    
    
    //Log in
    let client = Client(host: .StackOverflow)
    
    if !client.loggedIn {
        let email: String
        let password: String
        
        let env =  NSProcessInfo.processInfo().environment
        
        let envEmail = env["ChatBotEmail"]
        let envPassword = env["ChatBotPass"]
        
        if envEmail != nil {
            email = envEmail!
        }
        else {
            print("Email: ", terminator: "")
            email = readLine()!
        }
        
        if envPassword != nil {
            password = envPassword!
        }
        else {
            password = String(UTF8String: getpass("Password: "))!
        }
        
        do {
            try client.loginWithEmail(email, password: password)
        }
        catch Client.LoginError.LoginFailed(let message) {
            print("Login failed: \(message)")
            exit(EXIT_FAILURE)
        }
        catch {
            print("Login failed with error \(error).\nClearing cookies and retrying.")
            clearCookies()
            do {
                try client.loginWithEmail(email, password: password)
            }
            catch {
                print("Failed to log in!")
                exit(EXIT_FAILURE)
            }
        }
    }
    
    
    
    //Join the chat room
    let room = ChatRoom(client: client, roomID: 68414)  //SOCVR Testing Facility
    errorRoom = room
    let bot = ChatBot(room)
    room.delegate = bot
    try room.join()
    
    
    
    //Startup finished
    room.postMessage("[FireAlarm-Swift](//github.com/NobodyNada/FireAlarm/tree/swift) started.")
    
    
    
    //Run background tasks
    
    
    func inputMonitor() {
        repeat {
            if let input = readLine() {
                backgroundTasks.append(.HandleInput(input: input))
                dispatch_semaphore_signal(backgroundSemaphore)
            }
        } while true
    }
    
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), inputMonitor)
    
    
    repeat {
        //wait for a background task
        dispatch_semaphore_wait(backgroundSemaphore, DISPATCH_TIME_FOREVER)
        
        switch backgroundTasks.removeFirst() {
        case .HandleInput(let input):
            bot.chatRoomMessage(
                room,
                message: ChatMessage(
                    user: room.userWithID(0),
                    content: input,
                    id: nil
                ),
                isEdit: false
            )
        case .ShutDown(let reboot):
            //Wait for pending messages to be posted.
            while !room.messageQueue.isEmpty {
                sleep(1)
            }
            room.leave()
            
            
            if reboot {
                //Change to the old working directory.
                NSFileManager.defaultManager().changeCurrentDirectoryPath(originalWorkingDirectory)
                
                //Reload the program binary, which will restart the bot.
                execv(Process.arguments[0], Process.unsafeArgv)
            }
            //If a reboot fails, it will fall through to here & just shutdown instead.
            return
        }
    } while true
}

func halt(reboot reboot: Bool = false) {
    backgroundTasks.append(.ShutDown(reboot: reboot))
    dispatch_semaphore_signal(backgroundSemaphore)
}

func handleError(error: ErrorType, _ context: String? = nil) {
    let contextStr: String
    if context != nil {
        contextStr = " \(context!)"
    }
    else {
        contextStr = ""
    }
    
    let message1 = "An error (`\(String(reflecting: error.dynamicType))`) occured\(contextStr):"
    let message2 = String(error)
    
    if let room = errorRoom {
        room.postMessage(message1)
        room.postMessage("    " + message2)
    }
    else {
        fatalError("\(message1)\n\(message2)")
    }
}



try! main()