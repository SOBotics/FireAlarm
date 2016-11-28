import XCTest
@testable import FireAlarm

class FireAlarmTests: XCTestCase {
    func testExample() {
        // This is an example of a functional test case.
        // Use XCTAssert and related functions to verify your tests produce the correct results.
        XCTAssert(1)
    }


    static var allTests : [(String, (FireAlarmTests) -> () throws -> Void)] {
        return [
            ("testExample", testExample),
        ]
    }
}
