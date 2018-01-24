cmake_minimum_required(VERSION 2.8)

project(libtacom)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11")

include_directories("include")

add_library(libad5383 "src/ad5383.cpp")
add_library(libutils "src/utils.cpp")
add_library(libdevice "src/device.cpp")

add_library(libwaveform "src/waveform.cpp")
target_link_libraries(libwaveform libutils)

add_library(libwaveformobj "src/waveform_obj.cpp")
target_link_libraries(libwaveformobj libutils libdevice)

add_library(libalphabet "src/alphabet.cpp")
target_link_libraries(libalphabet libwaveformobj libdevice)


add_subdirectory("example")
add_subdirectory("determine-up")
add_subdirectory("demo_standard")
add_subdirectory("neutral")
add_subdirectory("caracterise2018")
#add_subdirectory("tests/test_mergethreadread")
#add_subdirectory("tests/test_readkey")
#add_subdirectory("tests/test_thread")
#add_subdirectory("tests/test_vectors")
#add_subdirectory("worldhaptics_demo")
#add_subdirectory("caracterise2018")
