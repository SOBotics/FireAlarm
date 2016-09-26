rm -rf update
(
cd update &&
git clone -b swift "https://github.com/NobodyNada/FireAlarm.git" update &&
swiftc -sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk -target x86_64-macosx10.11 -lz -lc++ -o ../FireAlarm FireAlarm/*.swift &&
git log --pretty=format:'%h' -n 1 > ../version-new.txt &&
cd .. &&
rm -rf update) ||
touch ../update-failure
