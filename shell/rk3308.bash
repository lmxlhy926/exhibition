#! /usr/bin/bash

#----------------Cmake--------------------------------
cmakePath=/usr/local/bin/cmake
gccPath=/usr/bin/aarch64-linux-gnu-gcc-8
cxxPath=/usr/bin/aarch64-linux-gnu-g++-8
sourceDir=/home/lhy/smarthome/exhibition
buildDir=${sourceDir}/build_arm64
ownMacro="-DLINUX_ARM64=true"
#-----------------------------------------------------

#----------------ADB----------------------------------
fromDir=/home/lhy/smarthome/exhibition/out/arm64
destDir=/data/changhong/edge_midware
#-----------------------------------------------------

#加载CMake
cmakeLoad(){
	${cmakePath} ${ownMacro} -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${gccPath} -DCMAKE_CXX_COMPILER=${cxxPath} -S ${sourceDir} -B ${buildDir}
}

#执行target
buildTargets(){
	cd ${buildDir}
	${cmakePath} --build ${buildDir} --target $1 -- -j 9
}

#推送站点到指定panel
pushSite2Panel(){
    IP=$1
    SITE=$2
    if [ ${SITE} == "query_site" -o ${SITE} == "config_site" -o ${SITE} == "ble_site" -o ${SITE} == "synergy_site" ]
    then
        /mnt/d/adb/adb.exe -s ${IP} push ${fromDir}/${SITE} ${destDir}
        /mnt/d/adb/adb.exe -s ${IP} shell chmod 0777 ${destDir}/${SITE}
        /mnt/d/adb/adb.exe -s ${IP} shell sync
        /mnt/d/adb/adb.exe -s ${IP} shell ls -lht /data/changhong/edge_midware/${SITE}
    fi
}

#param1：面板地址, param2: 存储目录
pullConfigLog(){
    address=$1
    fromPath=/data/changhong/edge_midware/lhy/configSiteLog.txt
    destDir=$2
    destPath="${destDir}\configSiteLog_${address}.txt"
    /mnt/d/adb/adb.exe -s ${address} pull ${fromPath} ${destPath}
}


if [ $1 == "build" ]
then
	read -p "targets: " -a targets
	cmakeLoad
	for target in ${targets[*]}
	do
		buildTargets ${target}
	done

elif [ $1 == "push" ]
then
	/mnt/d/adb/adb.exe devices
	echo "--------------------------"
    read -p "panel address: " -a panelAddressArray
	read -p "sites: " -a sites
    for panelAddress in ${panelAddressArray}
    do
        /mnt/d/adb/adb.exe connect ${panelAddress}
        for site in ${sites[*]}
        do
            pushSite2Panel ${panelAddress} ${site}
        done 
    done

elif [ $1 == "pull" ]
then
	/mnt/d/adb/adb.exe devices
	echo "--------------------------"
    read -p "panel address: " -a panelAddressArray
	read -p "pull file: configLog? " file
	if [ ${file} == "configLog" ]
	then
		for panelAddress in ${panelAddressArray[*]}
		do
			/mnt/d/adb/adb.exe connect ${panelAddress}
			pullConfigLog ${panelAddress} "D:\bywg\debug_3308"
		done
	fi

elif [ $1 == "shell" ]
then
	/mnt/d/adb/adb.exe devices
	echo "--------------------------"
	read -p "panelAddress: " panelAddress
    /mnt/d/adb/adb.exe -s ${panelAddress} shell

elif [ $1 == "export" ]
then
	#拷贝到指定目录
	echo "copy sites to /mnt/d/bywg/outbin/arm64 .........";
	cp /home/lhy/smarthome/exhibition/out/arm64/*site /mnt/d/bywg/outbin/arm64/
fi


