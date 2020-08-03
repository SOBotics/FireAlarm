// swift-tools-version:5.3
// You can downgrade to an earlier Swift version if needed (5.0 or newer should be OK)

import PackageDescription

let package = Package(
    name: "FireAlarm",
    platforms: [
        .macOS(.v10_16),    // You can downgrade to 10.12 or newer if needed
    ],
    dependencies: [
        .package(url: "git://github.com/SOBotics/SwiftChatSE",      from: "5.1.0"),
        .package(url: "git://github.com/SOBotics/SwiftStack",       from: "0.5.0"),
        .package(url: "git://github.com/krzyzanowskim/CryptoSwift", from: "0.9.0")
    ],
    targets: [
        .target(name: "FireAlarmCore", dependencies: ["SwiftChatSE", "SwiftStack", "CryptoSwift"]),
        .target(name: "Frontend", dependencies: ["FireAlarmCore", "SwiftChatSE", "CryptoSwift"]),
        .testTarget(name: "FireAlarmTests", dependencies: ["Frontend"])
    ]
)
