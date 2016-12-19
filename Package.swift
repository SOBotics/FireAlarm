import PackageDescription

let package = Package(
	name: "FireAlarm",
	dependencies: [
		.Package(url: "git://github.com/NobodyNada/SwiftChatSE", majorVersion: 2),
		.Package(url: "git://github.com/NobodyNada/SwiftStack", majorVersion: 0)
	]
)
