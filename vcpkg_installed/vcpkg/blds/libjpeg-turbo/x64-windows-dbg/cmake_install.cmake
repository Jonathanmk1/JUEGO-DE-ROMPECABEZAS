# Install script for directory: C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/src/3.1.1-23af682a76.clean

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/simd/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/sharedlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/src/md5/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/turbojpeg.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "bin" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/turbojpeg.dll")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/turbojpeg.dll" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/turbojpeg.dll")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "CMAKE_STRIP-NOTFOUND" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/turbojpeg.dll")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "bin" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE OPTIONAL FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/turbojpeg.pdb")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/pkgscripts/libjpeg.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/pkgscripts/libturbojpeg.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libjpeg-turbo" TYPE FILE FILES
    "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/pkgscripts/libjpeg-turboConfig.cmake"
    "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/pkgscripts/libjpeg-turboConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libjpeg-turbo/libjpeg-turboTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libjpeg-turbo/libjpeg-turboTargets.cmake"
         "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/CMakeFiles/Export/f0d506f335508d6549928070f26fb787/libjpeg-turboTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libjpeg-turbo/libjpeg-turboTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libjpeg-turbo/libjpeg-turboTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libjpeg-turbo" TYPE FILE FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/CMakeFiles/Export/f0d506f335508d6549928070f26fb787/libjpeg-turboTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libjpeg-turbo" TYPE FILE FILES "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/CMakeFiles/Export/f0d506f335508d6549928070f26fb787/libjpeg-turboTargets-debug.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/jonas/Downloads/rompecabezas-kinect-c/vcpkg_installed/vcpkg/blds/libjpeg-turbo/x64-windows-dbg/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
