cmake_minimum_required(VERSION 3.18.1)
project("modelinengine")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(native-lib SHARED
        native-lib.cpp
)

find_library(log-lib log)
find_library(GLESv3-lib GLESv3)
find_library(EGL-lib EGL)

target_link_libraries(native-lib
        ${log-lib}
        ${GLESv3-lib}
        ${EGL-lib}
)
