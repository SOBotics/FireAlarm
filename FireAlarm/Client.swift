//
//  Client.swift
//  FireAlarm
//
//  Created by Jonathan Keller on 8/27/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation

extension String {
    var urlEncodedString: String {
        let allowed = NSMutableCharacterSet(charactersInString: "+&")
        allowed.formUnionWithCharacterSet(NSCharacterSet.URLQueryAllowedCharacterSet())
        return self.stringByAddingPercentEncodingWithAllowedCharacters(allowed)!
    }
    
    init(urlParameters: [String:Any]) {
        var result = [String]()
        
        for (key, value) in urlParameters {
            result.append("\(key.urlEncodedString)=\(String(value).urlEncodedString)")
        }
        
        self.init(result.joinWithSeparator("&"))
    }
}

func + <K, V> (left: [K:V], right: [K:V]) -> [K:V] {
    var result = left
    for (k, v) in right {
        result[k] = v
    }
    return result
}

//http://stackoverflow.com/a/24052094/3476191
func += <K, V> (inout left: [K:V], right: [K:V]) {
    for (k, v) in right {
        left[k] = v
    }
}

class Client: NSObject, NSURLSessionDataDelegate {
    
    var session: NSURLSession!
    
    var loggedIn = false
    
    enum Host: String {
        case StackOverflow = "stackoverflow.com"
        case StackExchange = "stackexchange.com"
        case MetaStackExchange = "meta.stackexchange.com"
        
        var chatHost: String {
            return "chat." + rawValue
        }
        
        var url: NSURL {
            return NSURL(string: "https://" + rawValue)!
        }
        
        var chatHostURL: NSURL {
            return NSURL(string: "https://" + chatHost)!
        }
    }
    
    let host: Host
    
    enum RequestError: ErrorType {
        case InvalidURL
        case NotUTF8
    }
    
    func performRequest(request: NSURLRequest) throws -> (NSData, NSHTTPURLResponse) {
        let sema = dispatch_semaphore_create(0)
        
        var data: NSData!
        var resp: NSURLResponse!
        var error: NSError!
        
        session.dataTaskWithRequest(request) {inData, inResp, inError in
            (data, resp, error) = (inData, inResp, inError)
            dispatch_semaphore_signal(sema)
            }.resume()
        
        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER)
        
        guard let response = resp as? NSHTTPURLResponse where data != nil else {
            throw error
        }
        
        return (data, response)
    }
    
    func get(url: String) throws -> (NSData, NSHTTPURLResponse) {
        guard let nsUrl = NSURL(string: url) else {
            throw RequestError.InvalidURL
        }
        return try performRequest(NSURLRequest(URL: nsUrl))
    }
    
    func post(url: String, _ data: [String:Any]) throws -> (NSData, NSHTTPURLResponse) {
        guard let nsUrl = NSURL(string: url) else {
            throw RequestError.InvalidURL
        }
        let request = NSMutableURLRequest(URL: nsUrl)
        request.HTTPMethod = "POST"
        request.HTTPBody = String(urlParameters: data).dataUsingEncoding(NSUTF8StringEncoding)
        return try performRequest(request)
    }
    
    
    func performRequest(request: NSURLRequest) throws -> String {
        let (data, _) = try performRequest(request)
        guard let string = String(data: data, encoding: NSUTF8StringEncoding) else {
            throw RequestError.NotUTF8
        }
        return string
    }
    
    func get(url: String) throws -> String {
        let (data, _) = try get(url)
        guard let string = String(data: data, encoding: NSUTF8StringEncoding) else {
            throw RequestError.NotUTF8
        }
        return string
    }
    
    func post(url: String, _ fields: [String:Any]) throws -> String {
        let (data, _) = try post(url, fields)
        guard let string = String(data: data, encoding: NSUTF8StringEncoding) else {
            throw RequestError.NotUTF8
        }
        return string
    }
    
    
    override convenience init() {
        self.init(host: .StackOverflow)
    }
    
    init(host: Host) {
        self.host = host
        
        super.init()
        
        let configuration =  NSURLSessionConfiguration.defaultSessionConfiguration()
        
        session = NSURLSession(
            configuration: configuration,
            delegate: nil, delegateQueue: nil
        )
        
        //check if we're already logged in
        let request = NSURLRequest(URL: NSURL(string: "https://stackexchange.com/users/logout")!)
        
        let semaphore = dispatch_semaphore_create(0)
        
        let task = session.dataTaskWithRequest(request) {data, resp, error in
            guard let response = (resp as? NSHTTPURLResponse) where error == nil else {
                self.loggedIn = false
                dispatch_semaphore_signal(semaphore)
                return
            }
            
            self.loggedIn = response.statusCode == 200 && response.URL?.lastPathComponent == "logout"
            dispatch_semaphore_signal(semaphore)
        }
        
        task.resume()
        
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER)
    }
    
    enum LoginError: ErrorType {
        case AlreadyLoggedIn
        case LoginDataNotFound
        case LoginFailed(message: String)
    }
    
    private func getHiddenInputs(string: String) -> [String:String] {
        var result = [String:String]()
        
        let components = string.componentsSeparatedByString("<input type=\"hidden\"")
        
        for input in components[1..<components.count] {
            guard let nameStartIndex = input.rangeOfString("name=\"")?.endIndex else {
                continue
            }
            let nameStart = input.substringFromIndex(nameStartIndex)
            
            guard let nameEndIndex = nameStart.rangeOfString("\"")?.startIndex else {
                continue
            }
            let name = nameStart.substringToIndex(nameEndIndex)
            
            guard let valueStartIndex = nameStart.rangeOfString("value=\"")?.endIndex else {
                continue
            }
            let valueStart = nameStart.substringFromIndex(valueStartIndex)
            
            guard let valueEndIndex = valueStart.rangeOfString("\"")?.startIndex else {
                continue
            }
            
            let value = valueStart.substringToIndex(valueEndIndex)
            
            result[name] = value
        }
        
        return result
    }
    
    func loginWithEmail(email: String, password: String) throws {
        if loggedIn {
            throw LoginError.AlreadyLoggedIn
        }
        
        print("Logging in...")
        
        let loginPage: String = try get(post("https://stackexchange.com/users/signin",
            ["from" : "https://stackexchange.com/users/login/log-in"]
            ))
        
        let hiddenInputs = getHiddenInputs(loginPage)
        
        guard hiddenInputs["affId"] != nil && hiddenInputs["fkey"] != nil else {
            throw LoginError.LoginDataNotFound
        }
        
        let fields: [String:Any] = [
            "email":email,
            "password":password,
            "affId" : hiddenInputs["affId"]!,
            "fkey" : hiddenInputs["fkey"]!
        ]
        let (linkData, _) = try post(
            "https://openid.stackexchange.com/affiliate/form/login/submit", fields
        )
        
        let page = String(data: linkData, encoding: NSUTF8StringEncoding)!
        
        if let errorStartIndex = page.rangeOfString("<div class=\"error\"><p>")?.endIndex {
            let errorStart = page.substringFromIndex(errorStartIndex)
            let errorEndIndex = errorStart.rangeOfString("</p></div>")!.startIndex
            let error = errorStart.substringToIndex(errorEndIndex)
            
            throw LoginError.LoginFailed(message: error)
        }
        
        let linkStart = page.substringFromIndex(page.rangeOfString("<a href=\"")!.endIndex)
        let linkEndIndex = linkStart.rangeOfString("\"")!.startIndex
        let link = linkStart.substringToIndex(linkEndIndex)
        
        let (_,_) = try get(link)
        
        
        if !(host == .StackExchange) {
            //Login to host.
            let hostLoginURL = "https://\(host.rawValue)/users/login"
            let hostLoginPage: String = try get(hostLoginURL)
            guard let fkey = getHiddenInputs(hostLoginPage)["fkey"] else {
                throw LoginError.LoginDataNotFound
            }
            
            let (_,_) = try post(hostLoginURL, [
                "email" : email,
                "password" : password,
                "fkey" : fkey
                ]
            )
        }
    }
    
    
}
