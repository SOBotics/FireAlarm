import PackageDescription

let package = Package(
	name: "FireAlarm",
	dependencies: [
		.Package(url: "git://github.com/SOBotics/SwiftChatSE", majorVersion: 4),
		.Package(url: "git://github.com/SOBotics/SwiftStack", majorVersion: 0)
	]
)
