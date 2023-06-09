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
BUILD_TARGET_SOURCE_DIR=/home/lhy/smarthome/exhibition/out/arm64    # 站点所在目录
BUILD_TARGET_PANEL_DEST_DIR=/data/changhong/edge_midware            # 推送目录
PULL_LOG_DIR="D:\bywg\debug_3308"                                   # log保存目录
#-----------------------------------------------------
