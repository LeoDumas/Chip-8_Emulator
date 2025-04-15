cmake_minimum_required(VERSION 3.16)

project(chip_8_emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chip_8_emulator main.cpp
    ressources.md
    README.md)

include(GNUInstallDirs)
install(TARGETS chip_8_emulator
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
