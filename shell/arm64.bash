#! /usr/bin/bash

if [ $1 == "config" ]
then
	/usr/local/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc-8 -DCMAKE_CXX_COMPILER=/usr/bin/aarch64-linux-gnu-g++-8 -DLINUX_ARM64=true -S /home/lhy/smarthome/exhibition -B /home/lhy/smarthome/exhibition/build_arm64
else
	cd /home/lhy/smarthome/exhibition/build_arm64
	/usr/local/bin/cmake --build /home/lhy/smarthome/exhibition/build_arm64 --target $1 -- -j 9
fi	
