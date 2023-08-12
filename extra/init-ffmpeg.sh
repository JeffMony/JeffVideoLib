#!/bin/bash

# download ffmpeg source

FFMPEG_UPSTREAM=git@github.com:JeffMony/FFmpeg.git
TAG_NAME=n6.0

CUR_DIR=$(pwd)

SOURCE_DIR=${CUR_DIR}/ffmpeg

git clone ${FFMPEG_UPSTREAM} ${SOURCE_DIR}

cd ${SOURCE_DIR}

git checkout -b ${TAG_NAME} ${TAG_NAME}

cd -