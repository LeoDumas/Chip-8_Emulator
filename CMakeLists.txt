# cmake_minimum_required(VERSION 3.16)

# project(chip_8_emulator LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(SDL2 REQUIRED)

# add_executable(chip_8_emulator main.cpp
#     ressources.md
#     README.md)

# target_link_libraries(chip_8_emulator PRIVATE SDL2::SDL2)

# include(GNUInstallDirs)
# install(TARGETS chip_8_emulator
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )


cmake_minimum_required(VERSION 3.16)

project(chip_8_emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

set(SRC_FILES
    main.cpp
)

add_executable(chip_8_emulator ${SRC_FILES})

target_link_libraries(chip_8_emulator
    PRIVATE
      SDL2::SDL2
      SDL2::SDL2main
)

include(GNUInstallDirs)
install(TARGETS chip_8_emulator
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
