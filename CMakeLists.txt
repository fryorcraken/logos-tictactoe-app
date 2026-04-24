cmake_minimum_required(VERSION 3.14)
project(TictactoePlugin LANGUAGES CXX)

if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT.")
endif()

logos_module(
    NAME tictactoe
    SOURCES
        src/tictactoe_interface.h
        src/tictactoe_plugin.h
        src/tictactoe_plugin.cpp
        src/tictactoe_backend.h
        src/tictactoe_backend.cpp
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

find_package(Qt6 REQUIRED COMPONENTS Widgets Quick QuickWidgets)
target_link_libraries(tictactoe_module_plugin PRIVATE
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickWidgets
)

qt_add_resources(tictactoe_module_plugin "qml_resources"
    PREFIX "/"
    FILES
        src/qml/Main.qml
)
