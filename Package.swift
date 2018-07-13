// swift-tools-version:4.1

import PackageDescription

let package = Package(
    name: "FireAlarm",
    dependencies: [
        .package(url: "git://github.com/SOBotics/SwiftChatSE",      from: "5.0.7"),
        .package(url: "git://github.com/SOBotics/SwiftStack",       from: "0.5.0"),
        .package(url: "git://github.com/krzyzanowskim/CryptoSwift", from: "0.9.0")
    ],
    targets: [
        .target(name: "FireAlarm", dependencies: ["SwiftChatSE", "SwiftStack", "CryptoSwift"]),
        .testTarget(name: "FireAlarmTests", dependencies: ["FireAlarm"])
    ]
)
