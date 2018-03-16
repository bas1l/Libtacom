cmake_minimum_required(VERSION 2.8)

project(libtacom)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11")


###
# Include directory
###
include_directories("include")


###
# Static Libraries
###
add_library(config4cpp STATIC IMPORTED)
set_target_properties(config4cpp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libconfig4cpp.a)

###
# Dynamic Libraries
###
add_library(libad5383 "src/ad5383.cpp")
add_library(libGPIOClass "src/GPIOClass.cpp")
add_library(libutils "src/utils.cpp")
add_library(libdevice "src/device.cpp")

add_library(libwaveform "src/waveform.cpp")
target_link_libraries(libwaveform libutils libdevice)

add_library(libalphabet "src/alphabet.cpp")
target_link_libraries(libalphabet libwaveform libdevice)

add_library(config4HaptiComm "src/HaptiCommConfiguration.cpp")
target_link_libraries(config4HaptiComm config4cpp libdevice libwaveform)


###
# Executable Directory
###
add_subdirectory("example")
add_subdirectory("determine-up")
add_subdirectory("demo_standard")
add_subdirectory("neutral")
add_subdirectory("caracterise2018")
add_subdirectory("sinefunction_generator")
add_subdirectory("apparent_motion_experiment")
add_subdirectory("tests/read_values")
add_subdirectory("tests/HaptiCommConfig")

#add_subdirectory("tests/config4cpp/demos/simple-encapsulation")
#add_subdirectory("tests/test_mergethreadread")
#add_subdirectory("tests/test_readkey")
#add_subdirectory("tests/test_thread")
#add_subdirectory("tests/test_vectors")
#add_subdirectory("worldhaptics_demo")
#add_subdirectory("caracterise2018")

