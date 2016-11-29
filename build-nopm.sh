#!/bin/bash

#download and build SwiftChatSE
echo "Building SwiftChatSE..."
[[ -d SwiftChatSE ]] && rm -rf SwiftChatSE
git clone git://github.com/NobodyNada/SwiftChatSE || exit 1
pushd SwiftChatSE || exit 1
build-nopm.sh || (popd; exit 1)
popd


#build FireAlarm
echo "Building FireAlarm..." || exit 1
swiftc Sources/*.swift -L/usr/local/lib -ISwiftChatSE -LSwiftChatSE -lSwiftChatSE -o FireAlarm || exit 1
