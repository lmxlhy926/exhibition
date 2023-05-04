#! /usr/bin/bash

if [ $1 == "config" ]
then
	/usr/local/bin/cmake -DCMAKE_BUILD_TYPE=Release -DANDROID_ARM32=true -DANDROID=1 -DCMAKE_TOOLCHAIN_FILE=/home/lhy/install/android-ndk-r25b/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=24 -DANDROID_ABI=armeabi-v7a -S /home/lhy/smarthome/exhibition -B /home/lhy/smarthome/exhibition/build_android
else
	cd /home/lhy/smarthome/exhibition/build_android
	/usr/local/bin/cmake --build /home/lhy/smarthome/exhibition/build_android --target $1 -- -j 9
fi	