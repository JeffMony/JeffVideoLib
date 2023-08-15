# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(-DCMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -s -Wdeprecated-declarations")
set(-DCMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -s -Wdeprecated-declarations")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,ALL -O3 -flto -Wl,--gc-sections")

set(EXTRA_DIR ${CMAKE_CURRENT_LIST_DIR}/../third_party)

include_directories(${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/include)

set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src/main/cpp)

include_directories(${SOURCE_DIR})

include_directories(${EXTRA_DIR}/message_queue)

## libavcodec
add_library(libavcodec STATIC IMPORTED)
set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libavcodec.a)

## libavformat
add_library(libavformat STATIC IMPORTED)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libavformat.a)

## libavutil
add_library(libavutil STATIC IMPORTED)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libavutil.a)

## libswresample
add_library(libswresample STATIC IMPORTED)
set_target_properties(libswresample PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libswresample.a)

## libswscale
add_library(libswscale STATIC IMPORTED)
set_target_properties(libswscale PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libswscale.a)

add_library(libssl STATIC IMPORTED)
set_target_properties(libssl PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/openssl/${ANDROID_ABI}/lib/libssl.a)

add_library(libcrypto STATIC IMPORTED)
set_target_properties(libcrypto PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/openssl/${ANDROID_ABI}/lib/libcrypto.a)

add_library(libmessage_queue SHARED IMPORTED)
set_target_properties(libmessage_queue PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/src/main/nativeLibs/${ANDROID_ABI}/libmessage_queue.so)

file(
        GLOB
        muxer_source
        ${SOURCE_DIR}/video_processor.cc
        ${SOURCE_DIR}/media_env.c
        ${SOURCE_DIR}/media_jni.cc
)

add_library(
        media_muxer
        SHARED
        ${muxer_source})

target_link_libraries(
        media_muxer
        log
        z
        libmessage_queue
        libavformat
        libavcodec
        libavutil
        libswresample
        libswscale
        libssl
        libcrypto)