# Install script for directory: C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/src/3.1.1-23af682a76.clean/sharedlib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/pkgs/libjpeg-turbo_x64-windows/debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "OFF")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "CMAKE_OBJDUMP-NOTFOUND")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/jpeg.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "bin" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/jpeg62.dll")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/jpeg62.dll" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/jpeg62.dll")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "CMAKE_STRIP-NOTFOUND" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/jpeg62.dll")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "bin" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE OPTIONAL FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/jpeg62.pdb")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/sharedlib/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
