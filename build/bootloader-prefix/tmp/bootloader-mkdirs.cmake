# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.3.1/components/bootloader/subproject"
  "D:/Semestre_8/Software_embebido/workspace/Equipo1_T6/build/bootloader"
  "D:/Semestre_8/Software_embebido/workspace/Equipo1_T6/build/bootloader-prefix"
  "D:/Semestre_8/Software_embebido/workspace/Equipo1_T6/build/bootloader-prefix/tmp"
  "D:/Semestre_8/Software_embebido/workspace/Equipo1_T6/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Semestre_8/Software_embebido/workspace/Equipo1_T6/build/bootloader-prefix/src"
  "D:/Semestre_8/Software_embebido/workspace/Equipo1_T6/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Semestre_8/Software_embebido/workspace/Equipo1_T6/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Semestre_8/Software_embebido/workspace/Equipo1_T6/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
