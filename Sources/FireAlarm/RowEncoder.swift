//
//  RowEncoder.swift
//  FireAlarm
//
//  Created by NobodyNada on 7/14/17.
//

import Foundation
import SwiftChatSE

internal func nestedObject() -> Never {
    fatalError("Arrays and nested dictionaries may not be encoded into database rows")
}

public class RowEncoder {
    public init() {}
    
    public func encode(_ value: Codable) throws -> Row {
        let encoder = _RowEncoder()
        try value.encode(to: encoder)
        
        var columns = [DatabaseNativeType?]()
        var columnIndices = [String:Int]()
        for (key, value) in encoder.storage {
            columnIndices[key] = columns.count
            columns.append(value?.asNative)
        }
        
        return Row(columns: columns, columnNames: columnIndices)
    }
}


private class _RowEncoder: Encoder {
    var codingPath: [CodingKey] = []
    var userInfo: [CodingUserInfoKey : Any] = [:]
    
    var storage: [String:DatabaseType?] = [:]
    
    func container<Key>(keyedBy type: Key.Type) -> KeyedEncodingContainer<Key> where Key : CodingKey {
        return KeyedEncodingContainer(_KeyedEncoder<Key>(encoder: self, codingPath: codingPath))
    }
    
    func unkeyedContainer() -> UnkeyedEncodingContainer {
        nestedObject()
    }
    
    func singleValueContainer() -> SingleValueEncodingContainer {
        guard !codingPath.isEmpty else {
            fatalError("Root object for a database row must be a dictionary")
        }
        
        return _SingleValueContainer(encoder: self, key: codingPath.last!, codingPath: codingPath)
    }
}


private class _KeyedEncoder<K: CodingKey>: KeyedEncodingContainerProtocol {
    typealias Key = K
    let encoder: _RowEncoder
    let codingPath: [CodingKey]
    
    init(encoder: _RowEncoder, codingPath: [CodingKey]) {
        self.encoder = encoder
        self.codingPath = codingPath
    }
    
    func encodeNil(forKey key: K) throws {
        encoder.storage[key.stringValue] = nil as DatabaseType?
    }
    
    func encode(_ value: DatabaseType, forKey key: K) throws {
        encoder.storage[key.stringValue] = value
    }
    
    func encode<T>(_ value: T, forKey key: K) throws where T : Encodable {
        if let v = value as? DatabaseType {
            try encode(v, forKey: key)
            return
        }
        
        encoder.codingPath.append(key)
        try value.encode(to: encoder)
        encoder.codingPath.removeLast()
    }
    
    func nestedContainer<NestedKey: CodingKey>(keyedBy keyType: NestedKey.Type, forKey key: K) -> KeyedEncodingContainer<NestedKey> {
        nestedObject()
    }
    
    func nestedUnkeyedContainer(forKey key: K) -> UnkeyedEncodingContainer {
        nestedObject()
    }
    
    func superEncoder() -> Encoder {
        nestedObject()
    }
    
    func superEncoder(forKey key: K) -> Encoder {
        nestedObject()
    }
}


private class _SingleValueContainer: SingleValueEncodingContainer {
    let encoder: _RowEncoder
    let key: CodingKey
    let codingPath: [CodingKey]
    
    init(encoder: _RowEncoder, key: CodingKey, codingPath: [CodingKey]) {
        self.encoder = encoder
        self.key = key
        self.codingPath = codingPath
    }
    
    
    func encodeNil() throws {
        try encode(nil as DatabaseType?)
    }
    
    func encode(_ value: DatabaseType?) throws {
        encoder.storage[key.stringValue] = value
    }
    
    func encode<T>(_ value: T) throws where T : Encodable {
        fatalError("Single-value containers may only encode DatabaseTypes")
    }
}
