#! /usr/bin/bash

cmakePath=/usr/local/bin/cmake
gccPath=/usr/bin/aarch64-linux-gnu-gcc-8
cxxPath=/usr/bin/aarch64-linux-gnu-g++-8
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
