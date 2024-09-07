cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Specify the Android NDK
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 21)  # Adjust to your target API level
set(CMAKE_ANDROID_ARCH_ABI "armeabi-v7a")  # Adjust as necessary

# Path to the Android NDK
set(CMAKE_ANDROID_NDK "C:/Users/maxim/AppData/Local/Android/Sdk/ndk/26.1.10909125")

# Set the sysroot path
set(CMAKE_SYSROOT ${CMAKE_ANDROID_NDK}/platforms/android-${CMAKE_SYSTEM_VERSION}/arch-arm)

# Include directories for NDK headers
include_directories(${CMAKE_SYSROOT}/usr/include)

# Define the target executable or library
add_executable(getAcc src/main/cpp/getAcc.cpp)

# Link against system libraries
target_link_libraries(getAcc PRIVATE log android)
