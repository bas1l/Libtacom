# Install script for directory: /home/pi/Tacom/libtacom/executables

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/pi/Tacom/libtacom/executables/determine-up/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/demo_standard/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/neutral/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/caracterise2018/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/sinefunction_generator/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/apparent_motion_experiment/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/tests/read_values/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/tests/HaptiCommConfig/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/tests/sound_master/cmake_install.cmake")
  include("/home/pi/Tacom/libtacom/executables/tests/prototype/cmake_install.cmake")

endif()

