cmake_minimum_required(VERSION 3.10.2)
project("JavaInterface")

add_library(
        # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp
)

find_library(
        # Searches for the log library.
        log-lib
        log
)

target_link_libraries( # Specifies the libraries linked with the native libraries.)
        native-lib
        ${log-lib}
)