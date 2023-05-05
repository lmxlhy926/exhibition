#! /usr/bin/bash

if [ $1 == "all" ]
then
     /mnt/d/adb/adb.exe push /home/lhy/smarthome/exhibition/out/arm64/query_site /data/changhong/edge_midware
     /mnt/d/adb/adb.exe push /home/lhy/smarthome/exhibition/out/arm64/config_site /data/changhong/edge_midware
     /mnt/d/adb/adb.exe push /home/lhy/smarthome/exhibition/out/arm64/ble_site /data/changhong/edge_midware
     /mnt/d/adb/adb.exe push /home/lhy/smarthome/exhibition/out/arm64/synergy_site /data/changhong/edge_midware
    
elif [ $1 == "shell" ]
then 
    /mnt/d/adb/adb.exe shell
else 
    for site in $*
    do
        if [ $site == "query_site" ]
        then
            /mnt/d/adb/adb.exe push /home/lhy/smarthome/exhibition/out/arm64/query_site /data/changhong/edge_midware
        elif [  $site == "config_site" ]
        then
            /mnt/d/adb/adb.exe push /home/lhy/smarthome/exhibition/out/arm64/config_site /data/changhong/edge_midware
        elif [  $site == "ble_site" ]
        then
            /mnt/d/adb/adb.exe push /home/lhy/smarthome/exhibition/out/arm64/ble_site /data/changhong/edge_midware
        elif [  $site == "query_site" ]
        then
            /mnt/d/adb/adb.exe push /home/lhy/smarthome/exhibition/out/arm64/query_site /data/changhong/edge_midware
        fi
    done
fi

echo "---------------------------------------------------------------------------------"
/mnt/d/adb/adb.exe shell ls -lht /data/changhong/edge_midware/query_site /data/changhong/edge_midware/config_site /data/changhong/edge_midware/ble_site /data/changhong/edge_midware/synergy_site
echo "---------------------------------------------------------------------------------"
/mnt/d/adb/adb.exe shell ps -ef|grep site
echo "---------------------------------------------------------------------------------"