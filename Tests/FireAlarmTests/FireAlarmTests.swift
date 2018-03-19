import XCTest
@testable import FireAlarm
import SwiftChatSE

class FireAlarmTests: XCTestCase {
    func testTrainWreckerImages() {
        //Checks for broken trainwrecker links.
        
        let client = Client()
        for image in trains {
            let expectation = self.expectation(description: image)
            DispatchQueue.global().async {
                defer { expectation.fulfill() }
                do {
                    let (_, response) = try client.get(image)
                    if response.statusCode != 200 {
                        XCTFail("Trainwrecker image \(image) failed: \(response.statusCode)")
                    }
                    if !((response.allHeaderFields["Content-Type"] as? String)?.hasPrefix("image") ?? false) {
                        XCTFail("Trainwrecker image \(image) failed: \(response.allHeaderFields)")
                    }
                } catch {
                    XCTFail("Failed to fetch trainwrecker image \(image):\n\(error)")
                }
            }
        }
        
        waitForExpectations(timeout: 10)
    }
    
    
    static var allTests : [(String, (FireAlarmTests) -> () throws -> Void)] {
        return [
            ("testTrainWreckerImages", testTrainWreckerImages),
        ]
    }
}
