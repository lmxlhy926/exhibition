#! /usr/bin/bash

cmakePath=/usr/local/bin/cmake
gccPath=/home/lhy/install/RK3308/bin/aarch64-linux-gcc
cxxPath=/home/lhy/install/RK3308/bin/aarch64-linux-g++
sourceDir=/home/lhy/smarthome/exhibition
buildDir=${sourceDir}/build_arm64
ownMacro="-DLINUX_ARM64=true"

${cmakePath} ${ownMacro} -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${gccPath} -DCMAKE_CXX_COMPILER=${cxxPath} -S ${sourceDir} -B ${buildDir}

if [ $1 == "export" ]
then
	#拷贝到指定目录
	echo "-- copy to /mnt/d/bywg/outbin/arm64----";
	cp /home/lhy/smarthome/exhibition/out/arm64/*site /mnt/d/bywg/outbin/arm64/
else
	cd ${buildDir}
	${cmakePath} --build ${buildDir} --target $1 -- -j 9
fi

/home/lhy/install/RK3308/bin/aarch64-linux-gcc





