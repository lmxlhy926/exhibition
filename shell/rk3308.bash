#! /usr/bin/bash

# 加载配置文件
source ./property.sh

# 加载CMake
cmakeLoad(){
	${cmakePath} ${ownMacro} -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${gccPath} -DCMAKE_CXX_COMPILER=${cxxPath} -S ${sourceDir} -B ${buildDir}
}

# 执行target
buildTargets(){
	cd ${buildDir}
	${cmakePath} --build ${buildDir} --target $1 -- -j 9
}


adbCommon(){
	panelAddress=$1
	echo "*******************[panelAddress: ${panelAddress}]*******************"
	/mnt/d/adb/adb.exe -s ${panelAddress} shell byjs2023
}

adbSync(){
	panelAddress=$1
	/mnt/d/adb/adb.exe -s ${panelAddress} shell sync
}

adbPushSite(){
	panelAddress=$1										# 面板地址
	site=$2												# 目标站点
	sourceFilePath=${BUILD_TARGET_SOURCE_DIR}/${site}	# 源文件路径
	destDir=/data/changhong/edge_midware				# 目标文件夹
	
	adbCommon ${panelAddress}
	/mnt/d/adb/adb.exe -s ${panelAddress} push ${sourceFilePath} ${destDir}
	/mnt/d/adb/adb.exe -s ${panelAddress} shell chmod 0777 ${destDir}/${site}
    /mnt/d/adb/adb.exe -s ${panelAddress} shell ls -lht ${destDir}/${site}
}

pushSite2Panel(){
	panelAddress=$1
    site=$2
	if [ -f ${BUILD_TARGET_SOURCE_DIR}/${site} ]
	then
		adbPushSite ${panelAddress} ${site}
	fi
}

adbPull(){
	panelAddress=$1
	sourcePath=$2
	destPath=$3
	adbCommon ${panelAddress}
	/mnt/d/adb/adb.exe -s ${panelAddress} pull ${sourcePath} ${destPath}
}

pullQuerySiteLog(){
	panelAddress=$1	#面板ip地址
	destDir=$2		#存储目录
    sourcePath=/data/changhong/edge_midware/lhy/querySiteLog.txt
	destPath="${destDir}\querySiteLog_${panelAddress}.txt"
	adbPull ${panelAddress} ${sourcePath} ${destPath}
}

pullConfigSiteLog(){
    panelAddress=$1
	destDir=$2
    sourcePath=/data/changhong/edge_midware/lhy/configSiteLog.txt
    destPath="${destDir}\configSiteLog_${panelAddress}.txt"
    adbPull ${panelAddress} ${sourcePath} ${destPath}
}

pullBleSiteLog(){
	panelAddress=$1
	destDir=$2
    sourcePath=/data/changhong/edge_midware/lhy/bleMeshSiteLog.txt
    destPath="${destDir}\bleMeshSiteLog_${panelAddress}.txt"
    adbPull ${panelAddress} ${sourcePath} ${destPath}
}

pullSynergySiteLog(){
	panelAddress=$1
	destDir=$2
    sourcePath=/data/changhong/edge_midware/lhy/synergySiteLog.txt
    destPath="${destDir}\synergySiteLog_${panelAddress}.txt"
    adbPull ${panelAddress} ${sourcePath} ${destPath}
}

pullPaneInfo(){
	panelAddress=$1
	destDir=$2
    sourcePath=/data/changhong/edge_midware/panelConfig.txt
    destPath="${destDir}\panelConfig_${panelAddress}.txt"
    adbPull ${panelAddress} ${sourcePath} ${destPath}
}

blueHint(){
	message=$1
	echo -e -n "\033[34m${message}\033[0m"
}


if [ $1 == "build" ]
then
	blueHint "targets to build: "
	read -a targets
	cmakeLoad
	for target in ${targets[*]}
	do
		buildTargets ${target}
		echo "**************build target: ${target}**************"
	done

elif [ $1 == "connect" ]
then
	blueHint "input panelAddress: \n"
	panelAddressArray=`./addressExtract`	# 获取地址列表
	for panelAddress in ${panelAddressArray}
	do
		/mnt/d/adb/adb.exe connect ${panelAddress}
	done

elif [ $1 == "push" ]	#推送站点
then
	/mnt/d/adb/adb.exe devices
	blueHint "panels to push: \n"
	panelAddressArray=`./addressExtract`	# 获取地址列表
	blueHint "sites to push: "
	read -a sites
    for panelAddress in ${panelAddressArray[*]}
    do
        for site in ${sites[*]}
        do
            pushSite2Panel ${panelAddress} ${site}
        done 
		adbSync ${panelAddress}
    done

elif [ $1 == "pull" ]	#拉取log
then
	/mnt/d/adb/adb.exe devices
	blueHint "panel to pull: \n"
	panelAddressArray=`./addressExtract`	# 获取地址列表
	read -p "files to pull:" fileArray
	for panelAddress in ${panelAddressArray[*]}
	do
		for file in ${fileArray[*]}
		do
			if [ ${file} == "qlog" ]
			then
				pullQuerySiteLog ${panelAddress} ${PULL_LOG_DIR}
			elif [ ${file} == "clog" ]
			then
				pullConfigSiteLog ${panelAddress} ${PULL_LOG_DIR}
			elif [ ${file} == "blog" ]
			then
				pullBleSiteLog ${panelAddress} ${PULL_LOG_DIR}
			elif [ ${file} == "slog" ]
			then
				pullSynergySiteLog ${panelAddress} ${PULL_LOG_DIR}
			elif [ ${file} == "panelInfo" ]
			then
				pullPaneInfo ${panelAddress} ${PULL_LOG_DIR}
			fi
		done
	done

elif [ $1 == "shell" ]
then
	/mnt/d/adb/adb.exe devices
	read -p "panelAddress: " panelAddress
	/mnt/d/adb/adb.exe -s ${panelAddress} byjs2023
    /mnt/d/adb/adb.exe -s ${panelAddress} shell

elif [ $1 == "export" ]
then
	#拷贝到指定目录
	echo "copy sites to /mnt/d/bywg/outbin/arm64 .........";
	cp /home/lhy/smarthome/exhibition/out/arm64/*site /mnt/d/bywg/outbin/arm64/
	cp /home/lhy/smarthome/exhibition/out/arm64/nightStripTest /mnt/d/bywg/outbin/arm64/
fi

