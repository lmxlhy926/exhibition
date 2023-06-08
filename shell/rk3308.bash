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
BUILD_TARGET_SOURCE_DIR=/home/lhy/smarthome/exhibition/out/arm64
BUILD_TARGET_PANEL_DEST_DIR=/data/changhong/edge_midware
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


adbCommon(){
	ipAddress=$1
	echo "*******************[panelAddress: ${ipAddress}]*******************"
	/mnt/d/adb/adb.exe -s ${ipAddress} shell byjs2023
}

adbPushSite(){
	ipAddress=$1
	sourcePath=$2
	destDir=$3
	site=$4
	adbCommon ${ipAddress}
	/mnt/d/adb/adb.exe -s ${ipAddress} push ${sourcePath} ${destDir}
	/mnt/d/adb/adb.exe -s ${ipAddress} shell chmod 0777 ${destDir}/${site}
    /mnt/d/adb/adb.exe -s ${ipAddress} shell sync
    /mnt/d/adb/adb.exe -s ${ipAddress} shell ls -lht ${destDir}/${site}
}

pushSite2Panel(){
	ipAddress=$1
    site=$2
	sourcePath=${BUILD_TARGET_SOURCE_DIR}/${site}
	destDir=/data/changhong/edge_midware

	if [ ${site} == "query_site" -o ${site} == "config_site" -o ${site} == "ble_site" -o ${site} == "synergy_site" ]
    then
		adbPushSite ${ipAddress} ${sourcePath} ${destDir} ${site}
    fi
}

adbPull(){
	ipAddress=$1
	sourcePath=$2
	destPath=$3
	adbCommon ${ipAddress}
	/mnt/d/adb/adb.exe -s ${ipAddress} pull ${sourcePath} ${destPath}
}

pullQuerySiteLog(){
	address=$1	#面板ip地址
	destDir=$2	#存储目录
	destPath="${destDir}\querySiteLog_${address}.txt"
    sourcePath=/data/changhong/edge_midware/lhy/querySiteLog.txt
	adbPull ${address} ${sourcePath} ${destPath}
}

pullConfigSiteLog(){
    address=$1
	destDir=$2
    sourcePath=/data/changhong/edge_midware/lhy/configSiteLog.txt
    destPath="${destDir}\configSiteLog_${address}.txt"
    adbPull ${address} ${sourcePath} ${destPath}
}

pullBleSiteLog(){
	address=$1
	destDir=$2
    sourcePath=/data/changhong/edge_midware/lhy/bleMeshSiteLog.txt
    destPath="${destDir}\bleMeshSiteLog_${address}.txt"
    adbPull ${address} ${sourcePath} ${destPath}
}

pullSynergySiteLog(){
	address=$1
	destDir=$2
    sourcePath=/data/changhong/edge_midware/lhy/synergySiteLog.txt
    destPath="${destDir}\synergySiteLog_${address}.txt"
    adbPull ${address} ${sourcePath} ${destPath}
}

pullPaneInfo(){
	address=$1
	destDir=$2
    sourcePath=/data/changhong/edge_midware/panelConfig.txt
    destPath="${destDir}\panelConfig_${address}.txt"
    adbPull ${address} ${sourcePath} ${destPath}
}


if [ $1 == "build" ]
then
	read -p "targets: " -a targets
	cmakeLoad
	for target in ${targets[*]}
	do
		buildTargets ${target}
		echo "**************build target: ${target}**************"
	done

elif [ $1 == "connect" ]
then
	panelAddressArray=`./adbParse host ./querylog.txt`	# 获取地址列表
	for panelAddress in ${panelAddressArray}
	do
		/mnt/d/adb/adb.exe connect ${panelAddress}
	done

elif [ $1 == "push" ]	#推送站点
then
	/mnt/d/adb/adb.exe devices
	echo "--------------------------"
    read -p "panel to push: " -a panelAddressArray
	read -p "sites to push: " -a sites
    for panelAddress in ${panelAddressArray[*]}
    do
        for site in ${sites[*]}
        do
            pushSite2Panel ${panelAddress} ${site}
        done 
    done

elif [ $1 == "pushall" ]
then
	panelAddressArray=`./adbParse host ./querylog.txt`	# 获取地址列表
	read -p "sites to push: " -a sites
    for panelAddress in ${panelAddressArray[*]}
    do
        for site in ${sites[*]}
        do
            pushSite2Panel ${panelAddress} ${site}
        done 
    done

elif [ $1 == "pull" ]	#拉取log
then
	/mnt/d/adb/adb.exe devices
    read -p "panel to pull: " -a panelAddressArray
	read -p "files to pull:" fileArray
	for panelAddress in ${panelAddressArray[*]}
	do
		panelAddress=`echo ${panelAddress} | ./adbParse host`
		for file in ${fileArray[*]}
		do
			if [ ${file} == "qlog" ]
			then
				pullQuerySiteLog ${panelAddress} "D:\bywg\debug_3308"
			elif [ ${file} == "clog" ]
			then
				pullConfigSiteLog ${panelAddress} "D:\bywg\debug_3308"
			elif [ ${file} == "blog" ]
			then
				pullBleSiteLog ${panelAddress} "D:\bywg\debug_3308"
			elif [ ${file} == "slog" ]
			then
				pullSynergySiteLog ${panelAddress} "D:\bywg\debug_3308"
			elif [ ${file} == "panelInfo" ]
			then
				pullPaneInfo ${panelAddress} "D:\bywg\debug_3308"
			fi
		done
	done

elif [ $1 == "shell" ]
then
	/mnt/d/adb/adb.exe devices
	echo "--------------------------"
	read -p "panelAddress: " panelAddress
	/mnt/d/adb/adb.exe connect ${panelAddress}
	/mnt/d/adb/adb.exe -s ${panelAddress} byjs2023
    /mnt/d/adb/adb.exe -s ${panelAddress} shell

elif [ $1 == "export" ]
then
	#拷贝到指定目录
	echo "copy sites to /mnt/d/bywg/outbin/arm64 .........";
	cp /home/lhy/smarthome/exhibition/out/arm64/*site /mnt/d/bywg/outbin/arm64/
fi

