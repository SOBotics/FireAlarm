//
//  RowDecoder.swift
//  ORMTests
//
//  Created by NobodyNada on 7/17/17.
//

import Foundation
import SwiftChatSE

public class RowDecoder {
    public enum DecodingError: Error {
        case unexpectedNull
    }
    
    public init() {}
    
    public func decode<T: Codable>(_ type: T.Type, from row: Row) throws -> T{
        return try T.init(from: _RowDecoder(row: row))
    }
}


private class _RowDecoder: Decoder {
    var codingPath: [CodingKey] = []
    var userInfo: [CodingUserInfoKey : Any] = [:]
    
    var row: Row
    init(row: Row) {
        self.row = row
    }
    
    func container<Key>(keyedBy type: Key.Type) throws -> KeyedDecodingContainer<Key> where Key : CodingKey {
        return KeyedDecodingContainer<Key>(KeyedDecoder(decoder: self, codingPath: codingPath))
    }
    
    func unkeyedContainer() throws -> UnkeyedDecodingContainer {
        nestedObject()
    }
    
    func singleValueContainer() throws -> SingleValueDecodingContainer {
        guard !codingPath.isEmpty else {
            fatalError("Root object for a database row must be a dictionary")
        }
        
        return  SingleValueDecoder(decoder: self, key: codingPath.last!)
    }
}


private class KeyedDecoder<K: CodingKey>: KeyedDecodingContainerProtocol {
    var codingPath: [CodingKey]
    var allKeys: [K] {
        return decoder.row.columnNames.keys.compactMap { K(stringValue: $0) }
    }
    
    let decoder: _RowDecoder
    init(decoder: _RowDecoder, codingPath: [CodingKey]) {
        self.decoder = decoder
        self.codingPath = codingPath
    }
    
    
    func contains(_ key: K) -> Bool {
        return allKeys.contains { $0.stringValue == key.stringValue }
    }
    
    func decodeNil(forKey key: K) throws -> Bool {
        guard let columnIndex = decoder.row.columnNames[key.stringValue] else {
            throw DecodingError.keyNotFound(key, .init(
                codingPath: codingPath,
                debugDescription: "Key \(key) not found (valid keys are \(decoder.row.columnNames.keys)"
                )
            )
        }
        return decoder.row.columns[columnIndex] == nil
    }
    
    func decode<T: DatabaseType & Decodable>(_ type: T.Type, forKey key: K) throws -> T {
        guard let result = decoder.row.column(named: key.stringValue) as T? else {
            throw DecodingError.valueNotFound(T.self, DecodingError.Context(codingPath: codingPath, debugDescription: "key \(key) has a null value"))
        }
        return result
    }
    
    func decode<T: DatabaseType>(_ type: T.Type, forKey key: K) throws -> T {
        guard let result = decoder.row.column(named: key.stringValue) as T? else {
            throw DecodingError.valueNotFound(T.self, DecodingError.Context(codingPath: codingPath, debugDescription: "key \(key) has a null value"))
        }
        return result
    }
    
    func decode<T: Decodable>(_ type: T.Type, forKey key: K) throws -> T {
        if T.self == Data.self {    //Not sure why this is needed; seems like Data will only call this overload
            return try decode(Data.self, forKey: key) as! T
        }
        
        decoder.codingPath.append(key)
        let result = try type.init(from: decoder)
        decoder.codingPath.removeLast()
        return result
    }
    
    func nestedContainer<NestedKey>(keyedBy type: NestedKey.Type, forKey key: K) throws -> KeyedDecodingContainer<NestedKey> where NestedKey : CodingKey {
        nestedObject()
    }
    
    func nestedUnkeyedContainer(forKey key: K) throws -> UnkeyedDecodingContainer {
        nestedObject()
    }
    
    func superDecoder() throws -> Decoder {
        nestedObject()
    }
    
    func superDecoder(forKey key: K) throws -> Decoder {
        nestedObject()
    }
    
    typealias Key = K
}


private class SingleValueDecoder: SingleValueDecodingContainer {
    let decoder: _RowDecoder
    let key: CodingKey
    var codingPath: [CodingKey] { return decoder.codingPath }
    
    init(decoder: _RowDecoder, key: CodingKey) {
        self.decoder = decoder
        self.key = key
    }
    
    func decodeNil() -> Bool {
        if let index = decoder.row.columnNames[key.stringValue] {
            return decoder.row.columns[index] != nil
        }
        return true
    }
    
    func decode<T: DatabaseType & Decodable>(_ type: T.Type) throws -> T {
        guard let result = decoder.row.column(named: key.stringValue) as T? else {
            throw RowDecoder.DecodingError.unexpectedNull
        }
        
        return result
    }
    
    func decode<T: Decodable>(_ type: T.Type) throws -> T {
        fatalError("Single-value containers may only decode DatabaseTypes")
    }
}
