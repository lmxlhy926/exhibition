#! /usr/bin/bash

sourceDir=/home/lhy/smarthome/exhibition/out/arm64
destDir=/data/changhong/edge_midware
if [ $1 == "allbin" ]
then
    sites=(query_site config_site ble_site synergy_site)
    for site in ${sites[*]}
    do
        /mnt/d/adb/adb.exe push ${sourceDir}/${site} ${destDir}
        /mnt/d/adb/adb.exe shell chmod 0777 ${destDir}/${site}
    done
elif [ $1 == "shell" ]
then 
    /mnt/d/adb/adb.exe shell
else 
    for site in $*
    do
        if [ $site == "query_site" ]
        then
            /mnt/d/adb/adb.exe push ${sourceDir}/query_site ${destDir}
            /mnt/d/adb/adb.exe shell chmod 0777 ${destDir}/query_site
        elif [  $site == "config_site" ]
        then
            /mnt/d/adb/adb.exe push ${sourceDir}/config_site ${destDir}
            /mnt/d/adb/adb.exe shell chmod 0777 ${destDir}/config_site
        elif [  $site == "ble_site" ]
        then
            /mnt/d/adb/adb.exe push ${sourceDir}/ble_site ${destDir}
            /mnt/d/adb/adb.exe shell chmod 0777 ${destDir}/ble_site
        elif [  $site == "synergy_site" ]
        then
            /mnt/d/adb/adb.exe push ${sourceDir}/synergy_site ${destDir}
            /mnt/d/adb/adb.exe shell chmod 0777 ${destDir}/synergy_site
        fi
    done
fi

echo "---------------------------------------------------------------------------------"
/mnt/d/adb/adb.exe shell ls -lht /data/changhong/edge_midware/query_site /data/changhong/edge_midware/config_site /data/changhong/edge_midware/ble_site /data/changhong/edge_midware/synergy_site

