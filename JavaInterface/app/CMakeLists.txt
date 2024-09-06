# Minimum version of CMake required.
cmake_minimum_required(VERSION 3.10.2)

# Set the project name.
project("JavaInterface")

# Add the native shared library.
add_library(
        # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp
)

# Find the Android NKD libraries.
find_library(
        # Searches for the log library (for logging).
        log
        log-lib
)

find_library(
        # Searches for the android library.
        android
        android-lib
)

find_library(
        # Searches for the looper library (for sensor event loops).
        looper
        looper-lib
)

find_library(
        # Searches for the sensor library (for accelerometer data).
        sensor
        sensor-lib
)

# Link the native libraries with the NDK libraries (log, android, looper, sensor)
target_link_libraries(

        native-lib # Specifies the target library.
        ${log-lib} # Links the log library.
        ${android-lib} # Links the android library.
        ${looper-lib} # Links the looper library.
        ${sensor-lib} # Links the sensor library.
)