// swift-tools-version:4.2

import PackageDescription

let package = Package(
    name: "FireAlarm",
    dependencies: [
        .package(url: "git://github.com/SOBotics/SwiftChatSE",      from: "5.0.7"),
        .package(url: "git://github.com/SOBotics/SwiftStack",       from: "0.5.0"),
        .package(url: "git://github.com/krzyzanowskim/CryptoSwift", from: "0.9.0")
    ],
    targets: [
        .target(name: "FireAlarmCore", dependencies: ["SwiftChatSE", "SwiftStack", "CryptoSwift"]),
        .target(name: "FireAlarmFrontend", dependencies: ["FireAlarmCore", "SwiftChatSE", "CryptoSwift"]),
        .testTarget(name: "FireAlarmTests", dependencies: ["FireAlarmFrontend"])
    ]
)
