//
//  WebSocket.swift
//  FireAlarm
//
//  Created by NobodyNada on 11/17/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation
import libwebsockets

private func binToStr(_ data: Data) -> String? {
	var withNull = data
	if data[data.count - 1] != 0 {
		withNull.append(0)
	}
	
	return String(data: data, encoding: .utf8)
}

private func websocketCallback(
	_ context: OpaquePointer?,
	_ reason: lws_callback_reasons,
	_ user: UnsafeMutableRawPointer?,
	_ inData: UnsafeMutableRawPointer?,
	_ len: size_t
	) -> Int32 {
	
	//print(reason)
	
	let reasons = [	//The callback reasons to listen for.
		LWS_CALLBACK_CLIENT_RECEIVE,
		LWS_CALLBACK_CLIENT_WRITEABLE,
		LWS_CALLBACK_CLIENT_RECEIVE_PONG,
		LWS_CALLBACK_CLIENT_ESTABLISHED,
		LWS_CALLBACK_CLIENT_HTTP_WRITEABLE,
		LWS_CALLBACK_CLIENT_CONNECTION_ERROR,
		LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH,
		LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER,
		LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED,
		LWS_CALLBACK_CLOSED,
		/*LWS_CALLBACK_LOCK_POLL,
		LWS_CALLBACK_UNLOCK_POLL,
		LWS_CALLBACK_ADD_POLL_FD,
		LWS_CALLBACK_DEL_POLL_FD,
		LWS_CALLBACK_CHANGE_MODE_POLL_FD,
		LWS_CALLBACK_GET_THREAD_ID*/
	]
	if !reasons.contains(where: {$0 == reason}) {
		return 0
	}
	
	var ws: WebSocket! = user?.bindMemory(to: WebSocket.self, capacity: 1).pointee
	
	switch reason {
	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
		for socket in WebSocket.sockets {
			if socket.state == .connecting {
				if let headers = socket.customHeaders {
					guard headers.lengthOfBytes(using: .utf8) <= len else {
						print("Not enough buffer space for custom headers!")
						return 0
					}
					
					let ptr = inData!.bindMemory(to: UnsafeMutablePointer<CChar>.self, capacity: 1)
					let p = ptr.pointee
					strncpy(p, headers, len)
					ptr.pointee = p.advanced(by: Int(strlen(headers)))
				}
			}
		}
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		for socket in WebSocket.sockets {
			if socket.state == .connecting {
				ws = socket
				ws.opened()
				user?.bindMemory(to: WebSocket.self, capacity: 1).pointee = ws
				break
			}
		}
		
		
		
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		let message = inData.map { String(cString: $0.bindMemory(to: Int8.self, capacity: len)) }
		ws?.connectionError(message)
		
		
		
	case LWS_CALLBACK_CLIENT_RECEIVE:
		(inData?.bindMemory(to: UInt8.self, capacity: len)).map {
			ws?.received(data: $0, length: len)
		}
		
		
		
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		guard let (data, format) = ws.toWrite.first else {
			break
		}
		ws.toWrite.removeFirst()
		
		//SWIFT_LWS_PRE_PADDING and SWIFT_LWS_POST_PADDING are defined in my wrapper header "libwebsockets/lws.h"
		let size = Int(SWIFT_LWS_PRE_PADDING) + data.count + Int(SWIFT_LWS_POST_PADDING)
		
		let ptr = malloc(size).bindMemory(to: UInt8.self, capacity: size)
		
		let buf = UnsafeMutableBufferPointer<UInt8>(
			start: ptr.advanced(by: Int(SWIFT_LWS_PRE_PADDING)), count: data.count + Int(LWS_SEND_BUFFER_POST_PADDING)
		)
		
		
		
		let _ = data.copyBytes(to: buf)
		
		guard lws_write(ws.ws, buf.baseAddress, data.count, format) != -1 else {
			ws.state = .error
			ws.error = WebSocket.WebSocketError.writeFailed
			ws.errorHandler?(ws)
			return -1
		}
		
	case LWS_CALLBACK_CLOSED:
		ws?.connectionClosed()
		break
		
		
		
	default:
		break
	}
	
	
	
	if ws != nil {
		if ws.state == .disconnecting {
			return -1
		}
	}
	return 0
}

fileprivate func wsLog(level: Int32, buf: UnsafePointer<Int8>?) {
	buf.map {
		print("libwebsockets: " + String(cString: $0), terminator: "")
	}
}

public class WebSocket {
	public static var debugLoggingEnabled = false
	
	public enum WebSocketError: Error {
		case notURL(str: String)
		case invalidURL(url: URL)
		case creationFailed
		case lwsError(error: String?)
		case textNotUTF8(text: String)
		case writeFailed
	}
	
	public enum WebSocketState: Equatable {
		case disconnected
		case connecting
		case connected
		case disconnecting
		case error
	}
	
	
	
	public let origin: String?
	public let customHeaders: String?
	public let url: URL
	public let secure: Bool
	public var state: WebSocketState = .disconnected
	public fileprivate(set) var error: WebSocketError?
	
	public var openHandler: ((WebSocket) -> Void)?
	public var textHandler: ((WebSocket, String) -> Void)?
	public var binaryHandler: ((WebSocket, Data) -> Void)?
	public var errorHandler: ((WebSocket) -> Void)?
	public var closeHandler: ((WebSocket) -> Void)?
	
	public func onOpen(_ handler: ((WebSocket) -> Void)?) {
		openHandler = handler
	}
	
	public func onText(_ handler: ((WebSocket, String) -> Void)?) {
		textHandler = handler
	}
	
	public func onBinary(_ handler: ((WebSocket, Data) -> Void)?) {
		binaryHandler = handler
	}
	
	public func onError(_ handler: ((WebSocket) -> Void)?) {
		errorHandler = handler
	}
	
	public func onClose(_ handler: ((WebSocket) -> Void)?) {
		closeHandler = handler
	}
	
	
	
	convenience init(_ str: String, origin: String? = nil, headers: String? = nil) throws {
		guard let url = URL(string: str) else {
			throw WebSocketError.notURL(str: str)
		}
		
		try self.init(url, origin: origin, headers: headers)
	}
	
	
	
	init(_ url: URL, origin: String? = nil, headers: String? = nil) throws {
		if url.host == nil || url.scheme == "wss" {
			secure = false
		}
		else if url.scheme == "ws" {
			secure = false
		}
		else {
			throw WebSocketError.invalidURL(url: url)
		}
		
		guard url.host != nil else {
			throw WebSocketError.invalidURL(url: url)
			
		}
		
		self.url = url
		self.origin = origin
		self.customHeaders = headers
	}
	
	public func connect() throws {
		state = .connecting
		
		let hostSize = url.host!.lengthOfBytes(using: .utf8)
		let host = malloc(hostSize + 1).bindMemory(to: Int8.self, capacity: hostSize + 1)
		strcpy(host, url.host!)
		
		let urlPath = url.path + (url.query != nil ? ("?" + url.query!) : "")
		let pathSize = urlPath.lengthOfBytes(using: .utf8)
		let path = malloc(pathSize + 1).bindMemory(to: Int8.self, capacity: pathSize + 1)
		strcpy(path, urlPath)
		
		var originBuf: UnsafeMutablePointer<Int8>?
		origin.map {origin in
			let originSize = origin.lengthOfBytes(using: .utf8)
			originBuf = malloc(originSize + 1).bindMemory(to: Int8.self, capacity: originSize + 1)
			strcpy(originBuf!, origin)
		}
		
		
		var info = lws_client_connect_info()
		info.context = WebSocket.context
		info.address = UnsafePointer<Int8>(host)
		info.host = info.address
		info.path = UnsafePointer<Int8>(path)
		info.port = secure ? 443 : 80
		info.ssl_connection = secure ? 1 : 0
		info.origin = UnsafePointer<Int8>(originBuf)
		info.protocol = nil
		
		
		ws = lws_client_connect_via_info(&info)
		
		WebSocket.sockets.append(self)
		if WebSocket.sockets.count == 1 {
			WebSocket.runSockets()
		}
		
		guard ws != nil else {
			try error(.creationFailed)
		}
	}
	
	///Sends a text message across the socket.
	public func write(_ text: String) {
		guard let data = text.data(using: .utf8) else {
			print("text not UTF-8")
			return
		}
		
		doWrite(data, LWS_WRITE_TEXT)
	}
	
	
	///Sends a binary message across the socket.
	public func write(_ data: Data) {
		doWrite(data, LWS_WRITE_BINARY)
	}
	
	private func doWrite(_ data: Data, _ format: lws_write_protocol) {
		toWrite.append((data, format))
		
		lws_callback_on_writable(ws)
	}
	
	public func disconnect() {
		state = .disconnecting
		//force a callback so LWS knows to close the connection
		//lws_callback_all_protocol(WebSocket.context, WebSocket.protocols, Int32(LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION.rawValue))
		lws_callback_on_writable(ws)
	}
	
	
	fileprivate var ws: OpaquePointer?
	
	fileprivate var toWrite = [(Data, lws_write_protocol)]()
	fileprivate var requestedClose = false
	private var incomingData: Data?
	
	fileprivate static var sockets = [WebSocket]()
	
	fileprivate func connectionError(_ message: String?) {
		state = .error
		error = .lwsError(error: message)
	}
	
	fileprivate func opened() {
		state = .connected
		openHandler?(self)
	}
	
	fileprivate func connectionClosed() {
		state = .disconnected
		closeHandler?(self)
		if let index = WebSocket.sockets.index(where: { $0 === self }) {
			WebSocket.sockets.remove(at: index)
		}
	}
	
	fileprivate func received(data: UnsafePointer<UInt8>, length: Int) {
		let data = Data(bytes: data, count: length)
		if incomingData == nil {
			incomingData = data
		}
		else {
			incomingData!.append(data)
		}
		
		
		if lws_is_final_fragment(ws) != 0 {
			if lws_frame_is_binary(ws) == 0, let text = binToStr(incomingData!) {
				textHandler?(self, text)
			}
			else {
				binaryHandler?(self, incomingData!)
			}
			incomingData = nil
		}
	}
	
	
	
	private func error(_ error: WebSocketError) throws -> Never {
		state = .error
		self.error = error
		throw error
	}
	
	private static func _runSockets() {
		while !sockets.isEmpty {
			//every 100ms, check for websocket events
			lws_service(context, 100)
			//usleep(100000)
		}
	}
	
	#if os(macOS)
	
	@objc private static func _runSockets_thunk() {
		_runSockets()
	}
	
	private static func runSockets_selector() {
		Thread.detachNewThreadSelector(#selector(_runSockets_thunk), toTarget: self, with: nil)
	}
	
	#endif
	
	@available(OSX 10.12, *)
	private static func runSockets_block() {
		Thread.detachNewThread {
			_runSockets()
		}
	}
	
	static func runSockets() {
		#if os(Linux)
			runSockets_block()
		#else
			if #available(OSX 10.12, *) {
				runSockets_block()
			}
			else {
				runSockets_selector()
			}
		#endif
	}
	
	private static var _context: OpaquePointer?
	private static var _protocols: UnsafeMutablePointer<lws_protocols>?
	
	private static var protocols: UnsafeMutablePointer<lws_protocols> {
		if _protocols == nil {
			_protocols = UnsafeMutablePointer<lws_protocols>.allocate(capacity: 3)
			_protocols!.initialize(to: lws_protocols())
			
			let str = "http-only"
			let buf = UnsafeMutablePointer<Int8>.allocate(capacity: str.lengthOfBytes(using: .utf8))
			let _ = str.data(using: .utf8)!.copyBytes(
				to: UnsafeMutableBufferPointer<Int8>(start: buf, count: str.lengthOfBytes(using: .utf8))
			)
			let empty = UnsafeMutablePointer<Int8>.allocate(capacity: 1)
			empty.initialize(to: 0)
			
			_protocols!.advanced(by: 0).pointee.name = UnsafePointer<Int8>(buf)
			_protocols!.advanced(by: 1).pointee.name = UnsafePointer<Int8>(empty)
			_protocols!.advanced(by: 2).pointee.name = UnsafePointer<Int8>(empty)
			
			_protocols!.advanced(by: 0).pointee.callback = websocketCallback
			_protocols!.advanced(by: 1).pointee.callback = websocketCallback
			_protocols!.advanced(by: 2).pointee.callback = nil
			
			_protocols!.advanced(by: 0).pointee.id = 0
			_protocols!.advanced(by: 1).pointee.id = 0
			_protocols!.advanced(by: 2).pointee.id = 0
			
			_protocols!.advanced(by: 0).pointee.per_session_data_size = MemoryLayout<UnsafeMutablePointer<WebSocket>>.size
			_protocols!.advanced(by: 1).pointee.per_session_data_size = MemoryLayout<UnsafeMutablePointer<WebSocket>>.size
			_protocols!.advanced(by: 2).pointee.per_session_data_size = MemoryLayout<UnsafeMutablePointer<WebSocket>>.size
			
			_protocols!.advanced(by: 0).pointee.user = nil
			_protocols!.advanced(by: 1).pointee.user = nil
			_protocols!.advanced(by: 2).pointee.user = nil
			
			_protocols!.advanced(by: 0).pointee.rx_buffer_size = 0
			_protocols!.advanced(by: 1).pointee.rx_buffer_size = 0
			_protocols!.advanced(by: 2).pointee.rx_buffer_size = 0
		}
		return _protocols!
	}
	
	private static var context: OpaquePointer {
		if _context == nil {
			//Create a libwebsockets context.
			var info = lws_context_creation_info()
			
			info.port = CONTEXT_PORT_NO_LISTEN
			info.iface = nil
			info.protocols = UnsafePointer<lws_protocols>(protocols)
			info.extensions = nil
			info.ssl_cert_filepath = nil
			info.ssl_private_key_filepath = nil
			info.gid = -1
			info.uid = -1
			info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT.rawValue
			info.user = nil
			
			if debugLoggingEnabled {
				lws_set_log_level(255, wsLog)
			}
			
			
			_context = lws_create_context(&info)
			if _context == nil {
				fatalError("Failed to create websocket context!")
			}
		}
		
		return _context!
	}
}
