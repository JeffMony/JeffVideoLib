#!/bin/bash

# download ffmpeg source

FFMPEG_UPSTREAM=https://github.com/FFmpeg/FFmpeg
TAG_NAME=n5.1

CUR_DIR=$(pwd)

SOURCE_DIR=${CUR_DIR}/ffmpeg

git clone ${FFMPEG_UPSTREAM} ${SOURCE_DIR}

cd ${SOURCE_DIR}

git checkout -b ${TAG_NAME} ${TAG_NAME}

cd -