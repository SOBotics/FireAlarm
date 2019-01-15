//
//  startup.swift
//  FireAlarm
//
//  Created by NobodyNada on 4/18/17.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import Dispatch
import SwiftChatSE
import SwiftStack
import CryptoSwift

let commands: [Command.Type] = [
    CommandCheckThreshold.self, CommandSetThreshold.self, CommandCheckSites.self, CommandAddSite.self, CommandRemoveSite.self,
    CommandSay.self, CommandDeleteMessage.self,
    CommandHelp.self, CommandListRunning.self, CommandStop.self, CommandKill.self,
    CommandUpdate.self, CommandStatus.self, CommandPingOnError.self, CommandGitStatus.self,
    CommandCheckPrivileges.self, CommandPrivilege.self, CommandUnprivilege.self,
    CommandCheckPost.self, CommandQuota.self,
    CommandBlacklist.self, CommandGetBlacklist.self, CommandUnblacklist.self,
    CommandOptIn.self, CommandOptOut.self, CommandCheckNotification.self, CommandLeaveRoom.self,
    CommandLocation.self, CommandReport.self, CommandUnclosed.self, CommandTestBayesian.self,
    CommandWhy.self,
]

let trollCommands: [Command.Type] = [
    CommandSay.self, CommandDeleteMessage.self,
    CommandHelp.self, CommandListRunning.self, CommandStop.self, CommandKill.self,
    CommandUpdate.self, CommandStatus.self, CommandPingOnError.self, CommandGitStatus.self,
    CommandCheckPrivileges.self, CommandPrivilege.self, CommandUnprivilege.self,
    CommandCheckPost.self, CommandQuota.self,
    TrollCommandBlacklist.self, TrollCommandGetBlacklist.self, TrollCommandUnblacklist.self,
    TrollCommandEnable.self, TrollCommandDisable.self,
    CommandOptIn.self, CommandOptOut.self, CommandCheckNotification.self, CommandLeaveRoom.self,
    CommandLocation.self, CommandWhy.self
]


fileprivate var listener: ChatListener!

var reporter: Reporter!

var redunda: Redunda?
var bonfire: Bonfire?

//var apiClient = APIClient(proxyAddress: "127.0.0.1", proxyPort: 8080)
var apiClient = APIClient()

func main() throws {
    print("FireAlarm starting...")
    startTime = Date()
    afterTooManyErrors = {}
    addPrivileges()
    
    noUpdate = ProcessInfo.processInfo.arguments.contains("--noupdate")
    
    saveURL = saveDirURL
    
    apiClient.key = "HNA2dbrFtyTZxeHN6rThNg(("
    apiClient.defaultFilter = "!21PcIZT2MQcDURcNm2uJH"
    
    let client = Client()
    
    #if os(Linux)
    srand(UInt32(time(nil)))    //This is not cryptographically secure; it's just for train wrecking
    #endif
    
    if let redundaKey = try? loadFile("redunda_key.txt").trimmingCharacters(in: .whitespacesAndNewlines) {
        //standby until Redunda tells us not to
        redunda = Redunda(key: redundaKey, client: client, filesToSync: [
            "^reports\\.json$", "^room_\\d+_[a-z\\.]+\\.json$", "^secrets.json$",
            "^blacklists.json"
            ])
        
        var shouldStandby = false
        var isFirst = true
        repeat {
            do {
                try redunda!.sendStatusPing()
                if redunda!.shouldStandby {
                    shouldStandby = true
                    if isFirst {
                        print("FireAlarm started in standby mode.")
                    }
                    isFirst = false
                    sleep(30)
                } else {
                    shouldStandby = false
                }
            } catch {
                handleError(error, "while sending a status ping to Redunda")
            }
        } while shouldStandby
        if !isFirst {
            print("FireAlarm activating...")
        }
        
        do {
            try redunda!.downloadFiles()
        } catch {
            print("Could not download files!")
        }
    } else {
        fputs("warning: Could not load redunda_key.txt; running without Redunda.\n", stderr)
    }
    
    
    do {
        guard let secretsJSON = (try JSONSerialization.jsonObject(
            with: Data(contentsOf: saveDirURL.appendingPathComponent("secrets.json"))
            ) as? [String:String]) else {
                fatalError("Could not load secrets: secrets.json has an invalid format")
        }
        secrets = Secrets(json: secretsJSON)
    } catch {
        fatalError("Could not load secrets: \(error)")
    }
    
    if let bonfireKey = secrets.bonfireKey {
        bonfire = Bonfire(key: bonfireKey, client: client, host: "https://bonfire.sobotics.org")
    }
    
    print("Decompressing filter...")
    let result = run(command: "gunzip -c filter_static.sqlite.gz > filter_static.sqlite")
    if result.exitCode != 0 {
        print("Failed to decompress filter_static.sqlite.gz: \(result.stderr ?? "<no error>")")
    }
    
    //Log in
    let env =  ProcessInfo.processInfo.environment
    
    if !client.loggedIn {
        let email: String
        let password: String
        
        let envEmail = env["ChatBotEmail"]
        let envPassword = env["ChatBotPass"]
        
        if (envEmail ?? secrets.email) != nil {
            email = (envEmail ?? secrets.email)!
        }
        else {
            print("Email: ", terminator: "")
            email = readLine()!
        }
        
        if (envPassword ?? secrets.password) != nil {
            password = (envPassword ?? secrets.password)!
        }
        else {
            password = String(validatingUTF8: getpass("Password: "))!
        }
        
        do {
            try client.login(email: email, password: password)
        }
        catch {
            handleError(error, "while logging in")
            exit(EXIT_FAILURE)
        }
    }
    
    //Get the location
    if let rawLocation = redunda?.locationName {
        let components = rawLocation.components(separatedBy: "/")
        
        user = components.first ?? ""
        device = components.dropFirst().joined(separator: " ")
        
        location = "\(user)/\(device)"
        
        location = String(String.UnicodeScalarView(location.unicodeScalars.filter { !CharacterSet.newlines.contains($0) }))
        userLocation = location
        user = String(String.UnicodeScalarView(user.unicodeScalars.filter { !CharacterSet.newlines.contains($0) }))
        user = user.replacingOccurrences(of: " ", with: "")
        device = String(String.UnicodeScalarView(device.unicodeScalars.filter { !CharacterSet.newlines.contains($0) }))
        ping = " (cc @\(user))"
        userLocation = location
    } else if FileManager.default.fileExists (atPath: "location.txt") {
        do {
            let rawLocation = try loadFile ("location.txt")
            
            let components = rawLocation.components(separatedBy: "/")
            
            user = components.first ?? ""
            device = components.dropFirst().joined(separator: " ")
            
            location = "\(user)/\(device)"
            
            location = String(location.filter { !"\n".contains($0) })
            userLocation = location
            user = String(user.filter { !"\n".contains($0) })
            device = String(device.filter { !"\n".contains($0) })
            ping = " (cc @\(user))"
            userLocation = location
        } catch {
            print ("Location could not be loaded!")
        }
    } else {
        print ("Location could not be loaded!")
    }
    
    
    
    //Join the chat room
    let rooms: [ChatRoom]
    if let devString = env["DEVELOPMENT"], let devRoom = Int(devString) {
        let devServer: ChatRoom.Host
        if let devServerString = env["DEVELOPMENT_HOST"] {
            switch devServerString.lowercased() {
            case "chat.so":
                devServer = .stackOverflow
            case "chat.se":
                devServer = .stackExchange
            case "chat.mse":
                devServer = .metaStackExchange
            default:
                fatalError("DEVELOPMENT_HOST contains an invalid value; accepted values are chat.SO, chat.SE, and chat.mSE (case-insensitive)")
            }
        } else {
            devServer = .stackOverflow
        }
        
        rooms = [ChatRoom(client: client, host: devServer, roomID: devRoom)]
        development = true
    }
    else {
        rooms = [
            ChatRoom(client: client, host: .stackOverflow, roomID: 123602), //FireAlarm Development
            ChatRoom(client: client, host: .stackOverflow, roomID: 111347), //SOBotics
            ChatRoom(client: client, host: .stackOverflow, roomID: 41570),  //SO Close Vote Reviewers
            //ChatRoom(client: client, host: .stackExchange, roomID: 54445),	//SEBotics
        ]
        
        development = false
    }
    let trollRooms: [ChatRoom] = [ChatRoom(client: client, host: .stackExchange, roomID: 54445)] //SEBotics
    
    try (rooms + trollRooms).forEach {try $0.loadUserDB()}
    
    afterTooManyErrors = {
        print("Too many errors; aborting...")
        abort()
    }
    errorRoom = rooms.first!
    
    
    listener = ChatListener(commands: commands)
    listener.onShutdown { shutDown(reason: $0, rooms: rooms + trollRooms) }
    rooms.forEach { room in
        let trainWrecker = TrainWrecker(room: room)
        room.onMessage { message, isEdit in
            let content = message.content.lowercased()
            if (content == "@bots alive") {
                do {
                    try CommandStatus(listener: listener, message: message, arguments: []).run()
                } catch {
                    handleError(error, "while handling '@bots alive'")
                }
            }
            else if message.room.roomID == 111347 && ["🚆", "🚅", "🚂", "🚊"].contains(content) {
                room.postMessage("[🚃](https://www.youtube.com/watch?v=dQw4w9WgXcQ)")
            }
            
            if (message.room.roomID == 111347 || message.room.roomID == 123602) {
                if !isEdit { trainWrecker.process(message: message) }
            }
            
            listener.processMessage(room, message: message, isEdit: isEdit)
        }
    }
    
    let trollListener = ChatListener(commands: trollCommands)
    trollListener.onShutdown(listener.stop)
    trollRooms.forEach { room in
        room.onMessage { message, isEdit in
            trollListener.processMessage(room, message: message, isEdit: isEdit)
        }
    }
    
    try rooms.forEach { try $0.join() }
    try trollRooms.forEach { try $0.join() }
    
    currentVersion = getCurrentVersion()
    shortVersion = getShortVersion(currentVersion)
    versionLink = getVersionLink(currentVersion)
    
    rooms.first?.postMessage("[ [\(botName)](\(stackAppsLink)) ] FireAlarm started at revision [`\(shortVersion)`](\(versionLink)) on \(location).")
    
    //Load the filter
    reporter = Reporter(rooms: rooms, trollRooms: trollRooms)
    try reporter.postFetcher.start()
    
    errorsInLast30Seconds = 0
    afterTooManyErrors = {
        print("Too many errors; aborting...")
        abort()
    }
    
    
    scheduleBackgroundTasks(rooms: rooms, listener: listener)
}

private func sendUpdateBroadcast(commit: String) throws {
    //Send the update notification.
    if let token = secrets.githubWebhookSecret, let r = redunda {
        let payload = [
            "token": token,
            "commit": commit,
            ]
        let data = try JSONSerialization.data(withJSONObject: payload)
        let (_, _) = try r.client.post(
            "https://redunda.sobotics.org/bots/6/events/update_to?broadcast=true",
            data: data,
            contentType: "application/json"
        )
    }
}


private func shutDown(reason: ChatListener.StopReason, rooms: [ChatRoom]) {
    let shouldReboot = reason == .reboot || reason == .update
    
    reporter.postFetcher.stop()
    
    //Wait for pending messages to be posted.
    for room in rooms {
        while !room.messageQueue.isEmpty {
            sleep(1)
        }
    }
    while reporter != nil && !(reporter.postFetcher.ws.state == .disconnected || reporter.postFetcher.ws.state == .error) {
        sleep(1)
    }
    
    save(rooms: rooms)
    
    rooms.forEach { $0.leave() }
    
    if shouldReboot {
        let command = "/usr/bin/env swift run -c release"
        let args = command.components(separatedBy: .whitespaces)
        let argBuffer = UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>.allocate(capacity: args.count + 1)
        for i in 0..<args.count {
            argBuffer[i] = .allocate(capacity: args[i].utf8CString.count)
            _ = args[i].utf8CString.withUnsafeBufferPointer {
                strcpy(argBuffer[i]!, $0.baseAddress!)
            }
        }
        argBuffer[args.count] = nil
        execv("/usr/bin/env", argBuffer)
        //If the exec failed, exit 1 for my script, which automatically reboots on crashes.
        exit(1)
    }
    
    exit(0)
}
