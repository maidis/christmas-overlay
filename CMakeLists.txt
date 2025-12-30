cmake_minimum_required(VERSION 3.16)

project(ChristmasOverlay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Hint for custom Qt installation
list(APPEND CMAKE_PREFIX_PATH "/Users/anilozbek/Qt/6.10.1/macos")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

add_executable(ChristmasOverlay
    src/main.cpp
    src/treewidget.cpp
    src/treewidget.h
    src/snowoverlay.cpp
    src/snowoverlay.h
)

target_link_libraries(ChristmasOverlay PRIVATE Qt6::Widgets Qt6::Core)

if(APPLE)
    set_target_properties(ChristmasOverlay PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.christmasoverlay"
    )
endif()
