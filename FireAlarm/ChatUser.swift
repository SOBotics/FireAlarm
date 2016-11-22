//
//  ChatUser.swift
//  FireAlarm
//
//  Created by NobodyNada on 8/28/16.
//  Copyright © 2016 NobodyNada. All rights reserved.
//

import Foundation



open class ChatUser: CustomStringConvertible {
    
    open let id: Int
    
    fileprivate var _name: String?
    fileprivate var _isMod: Bool?
    fileprivate var _isRO: Bool?
	
	///Custom per-user persistent storage.  Must be serializable by JSONSerialization!
	open var info: [String:Any] = [:]
	
	public struct Privileges: OptionSet {
		public let rawValue: UInt
		
		public init(rawValue: Privileges.RawValue) {
			self.rawValue = rawValue
		}
		
		public static let owner = Privileges(rawValue: 1 << 0)
		
		
		
		public func has(privileges required: Privileges) -> Bool {
			return required.isSuperset(of: self)
		}
		
		public func missing(from required: Privileges) -> Privileges {
			return required.subtracting(self)
		}
		
		public var names: [String] {
			var raw = rawValue
			var shifts = 0
			var result = [String]()
			
			
			while raw != 0 {
				let priv = Privileges(rawValue: (raw << Privileges.RawValue(shifts)) & Privileges.RawValue(1 << shifts))
				if priv.rawValue != 0 {
					result.append(Privileges.name(of: priv))
				}
				
				raw >>= 1
				shifts += 1
			}
			
			
			return result
		}
		
		
		
		
		
		public static func add(name: String, for privilege: Privileges) {
			assertOne(privilege)
			privilegeNames[privilege.rawValue] = name
		}
		
		public static func name(of privilege: Privileges) -> String {
			assertOne(privilege)
			return privilegeNames[privilege.rawValue] ?? "<unnamed privilege>"
		}
		
		
		
		
		
		public private(set) static var privilegeNames: [Privileges.RawValue:String] = { [owner.rawValue:"Owner"] }()
		
		private static func assertOne(_ privileges: Privileges) {
			//count the number of ones in the binary representation of privilege's raw value
			var raw = privileges.rawValue
			var ones = 0
			
			while raw != 0 {
				if (raw & 1) != 0 {
					ones += 1
				}
				raw >>= 1
			}
			
			if ones != 1 {
				fatalError("privilege must contain exactly one privilege")
			}
		}
	}
    
    open var name: String {
        get {
            if let n = _name {
                return n
            }
            else {
                room.lookupUserInformation()
                return _name ?? "<unkown user \(id)>"
            }
        }
        set {
            _name = newValue
        }
    }
    
    open var isMod: Bool {
        get {
            if let i = _isMod {
                return i
            }
            else {
                room.lookupUserInformation()
                return _isMod ?? false
            }
        }
        set {
            _isMod = newValue
        }
    }
    
    open var isRO: Bool {
        get {
            if let i = _isRO {
                return i
            }
            else {
                room.lookupUserInformation()
                return _isRO ?? false
            }
        }
        set {
            _isRO = newValue
        }
    }
    
    open var description: String {
        return name
    }
	
	open var privileges: Privileges = []
    
    open let room: ChatRoom
    
    public init(room: ChatRoom, id: Int, name: String? = nil) {
        self.room = room
        self.id = id
        _name = name
    }
	
	public func has(privileges required: Privileges) -> Bool {
		return isMod || isRO || id == 0 || privileges.has(privileges: required)
	}
	
	public func missing(from required: Privileges) -> Privileges {
		return (isMod || isRO || id == 0) ? [] : privileges.missing(from: required)
	}
}
