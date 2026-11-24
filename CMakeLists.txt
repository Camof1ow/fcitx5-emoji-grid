cmake_minimum_required(VERSION 3.16)
project(fcitx5-emoji-grid VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5Module REQUIRED COMPONENTS DBus)

add_library(emojigrid SHARED emojigrid.cpp)
target_link_libraries(emojigrid Fcitx5::Core Fcitx5::Module::DBus)
install(TARGETS emojigrid DESTINATION lib/fcitx5)

# Library 에 설치 경로(스토어 경로)를 절대경로로 적어, fcitx5 의 FCITX_ADDON_DIRS 밖에서도 읽히게 한다.
configure_file(emojigrid.conf.in emojigrid.conf @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/emojigrid.conf DESTINATION share/fcitx5/addon)

install(PROGRAMS emoji-grid-ui DESTINATION bin)
