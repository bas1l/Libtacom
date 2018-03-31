cmake_minimum_required(VERSION 2.8)

project(libtacom)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11")


###
# Include directory
###
include_directories("include")


### EXTERNAL LIBRARIES ###
# Static Libraries
add_library(config4cpp STATIC IMPORTED)
set_target_properties(config4cpp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/external_lib/libconfig4cpp.a)
# Dynamic Libraries
add_library(wavFileReader "src/wav_file.cpp")
#add_subdirectory("external_lib/sound_master")



### OUR LIBRARIES ###
# Dynamic Libraries
add_library(libad5383 "src/ad5383.cpp")
add_library(libGPIOClass "src/GPIOClass.cpp")
add_library(libutils "src/utils.cpp")
add_library(libdevice "src/device.cpp")

add_library(libwaveform "src/waveform.cpp")
target_link_libraries(libwaveform libutils libdevice wavFileReader)

add_library(libalphabet "src/alphabet.cpp")
target_link_libraries(libalphabet libwaveform libdevice)

add_library(config4HaptiComm "src/HaptiCommConfiguration.cpp")
target_link_libraries(config4HaptiComm config4cpp libdevice libwaveform)


###
# Executable Directory
###
add_subdirectory("executables")
