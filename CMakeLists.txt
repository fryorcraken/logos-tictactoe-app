cmake_minimum_required(VERSION 3.14)
project(TictactoePlugin LANGUAGES CXX)

if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT.")
endif()

logos_module(
    NAME tictactoe
    REP_FILE src/tictactoe.rep
    SOURCES
        src/tictactoe_interface.h
        src/tictactoe_plugin.h
        src/tictactoe_plugin.cpp
)
