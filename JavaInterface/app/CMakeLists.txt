cmake_minimum_required(VERSION 3.22.1)

# Set the project name.
project("JavaInterface")

# Add the native shared library.
add_library(
        getSensorData
        SHARED
        src/main/cpp/getSensorData.cpp  # Your source file(s)
)

# Find the NDK libraries you need.
find_library(
        log-lib       # Variable to store the path
        log           # The actual library name (log from NDK)
)

find_library(
        android-lib   # Variable for android library path
        android       # The actual Android library name
)

# Link the native shared library with the NDK libraries.
target_link_libraries(
        getSensorData
        ${log-lib}    # Links the log library
        ${android-lib}# Links the Android library
)
