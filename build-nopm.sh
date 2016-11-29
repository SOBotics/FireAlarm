#!/bin/bash

#download and build SwiftChatSE
echo "Building SwiftChatSE..."
#[[ -d SwiftChatSE ]] && rm -rf SwiftChatSE
#git clone git://github.com/NobodyNada/SwiftChatSE
pushd SwiftChatSE
build-nopm.sh
popd


#build FireAlarm
echo "Building FireAlarm..."
swiftc Sources/*.swift -L/usr/local/lib -ISwiftChatSE -LSwiftChatSE -lSwiftChatSE -o FireAlarm
