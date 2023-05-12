#! /usr/bin/bash



for file in $*
do
    if [ $file == "" ]
    then
        /mnt/d/adb/adb.exe pull /data/changhong/edge_midware/data $1 
        /mnt/d/adb/adb.exe pull /data/changhong/edge_midware/panelConfig.txt $1
    elif [ $file == ""]
    then
        /mnt/d/adb/adb.exe pull /data/changhong/edge_midware/lhy $1
        
    then

    elif [ $file == ""]
    then

    elif [ $file == ""]
    then


    fi

done







