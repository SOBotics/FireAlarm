import PackageDescription

let package = Package(
	name: "FireAlarm",
	dependencies: [
		.Package(url: "git://github.com/NobodyNada/SwiftChatSE", majorVersion: 0)
	]
)
