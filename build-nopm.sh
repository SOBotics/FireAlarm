#!/bin/bash

#download libraries
[[ -d SwiftChatSE ]] && rm -rf SwiftChatSE
git clone git://github.com/NobodyNada/SwiftChatSE -b rpi || exit 1

[[ -d SwiftStack ]] && rm -rf SwiftStack
git clone git://github.com/NobodyNada/SwiftStack || exit 1

#build SwiftChatSE
echo "Building SwiftChatSE..."
pushd SwiftChatSE || exit 1
./build-nopm.sh || (popd; exit 1)
popd

#build SwiftStack
echo "Building SwiftStack..."
pushd SwiftStack || exit 1
./build-nopm.sh || (popd; exit 1)
popd


#build FireAlarm
echo "Building FireAlarm..." || exit 1
swiftc Sources/*.swift -L/usr/local/lib -ISwiftChatSE -ISwiftStack -LSwiftChatSE -LSwiftStack -lSwiftChatSE -lSwiftStack -o FireAlarm || exit 1
