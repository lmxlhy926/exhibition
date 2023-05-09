#! /usr/bin/bash

if [ $1 == "configure" ]
then
	/usr/local/bin/cmake -DCMAKE_BUILD_TYPE=Release -DANDROID_ARM32=true -DANDROID=1 -DCMAKE_TOOLCHAIN_FILE=/home/lhy/install/android-ndk-r25b/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=24 -DANDROID_ABI=armeabi-v7a -S /home/lhy/smarthome/exhibition -B /home/lhy/smarthome/exhibition/build_android
elif [ $1 == "export" ]
then
	#拷贝到指定目录
	echo "-- copy to /mnt/d/bywg/outbin/android32----";
	cp /home/lhy/smarthome/exhibition/out/android32/*site /mnt/d/bywg/outbin/android32/
else
	cd /home/lhy/smarthome/exhibition/build_android
	/usr/local/bin/cmake --build /home/lhy/smarthome/exhibition/build_android --target $1 -- -j 9
	#strip去掉调试信息
	/home/lhy/install/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip /home/lhy/smarthome/exhibition/out/android32/query_site
	/home/lhy/install/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip /home/lhy/smarthome/exhibition/out/android32/config_site
	/home/lhy/install/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip /home/lhy/smarthome/exhibition/out/android32/ble_site
	/home/lhy/install/android-ndk-r25b/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip /home/lhy/smarthome/exhibition/out/android32/synergy_site
fi	