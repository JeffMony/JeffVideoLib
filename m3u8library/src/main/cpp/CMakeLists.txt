# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(-DCMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -s -Wdeprecated-declarations")
set(-DCMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -s -Wdeprecated-declarations")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,ALL -O3 -flto -Wl,--gc-sections")

set(EXTRA_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../extra)

include_directories(${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/include)

## libavcodec.so
add_library(libavcodec STATIC IMPORTED)
set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libavcodec.a)

## libavformat.so
add_library(libavformat STATIC IMPORTED)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libavformat.a)

## libavutil.so
add_library(libavutil STATIC IMPORTED)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libavutil.a)

## libswresample.so
add_library(libswresample STATIC IMPORTED)
set_target_properties(libswresample PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libswresample.a)

## libswscale.so
add_library(libswscale STATIC IMPORTED)
set_target_properties(libswscale PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/ffmpeg/${ANDROID_ABI}/lib/libswscale.a)

add_library(libssl STATIC IMPORTED)
set_target_properties(libssl PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/openssl/${ANDROID_ABI}/lib/libssl.a)

add_library(libcrypto STATIC IMPORTED)
set_target_properties(libcrypto PROPERTIES IMPORTED_LOCATION ${EXTRA_DIR}/output/openssl/${ANDROID_ABI}/lib/libcrypto.a)

include_directories(.)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        media_muxer

        # Sets the library as a shared library.
        SHARED

        video_processor.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        media_muxer
        log
        z
        libavformat
        libavcodec
        libavutil
        libswresample
        libswscale
        libssl
        libcrypto)