cmake_minimum_required(VERSION 3.14)
project(TictactoePlugin LANGUAGES CXX)

if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT.")
endif()

# Protobuf for multiplayer wire format.
find_package(Protobuf REQUIRED)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto_gen)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(PROTO_SRC ${PROTO_GEN_DIR}/tictactoe.pb.cc)
set(PROTO_HDR ${PROTO_GEN_DIR}/tictactoe.pb.h)

add_custom_command(
    OUTPUT ${PROTO_SRC} ${PROTO_HDR}
    COMMAND protobuf::protoc
        --cpp_out=${PROTO_GEN_DIR}
        -I${PROTO_DIR}
        ${PROTO_DIR}/tictactoe.proto
    DEPENDS ${PROTO_DIR}/tictactoe.proto
    COMMENT "Generating protobuf sources for tictactoe"
)

logos_module(
    NAME tictactoe
    SOURCES
        src/tictactoe_interface.h
        src/tictactoe_plugin.h
        src/tictactoe_plugin.cpp
        src/tictactoe_backend.h
        src/tictactoe_backend.cpp
        ${PROTO_SRC}
        ${PROTO_HDR}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

find_package(Qt6 REQUIRED COMPONENTS Widgets Quick QuickWidgets)
target_include_directories(tictactoe_module_plugin PRIVATE ${PROTO_GEN_DIR})
target_link_libraries(tictactoe_module_plugin PRIVATE
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickWidgets
    protobuf::libprotobuf
)

qt_add_resources(tictactoe_module_plugin "qml_resources"
    PREFIX "/"
    FILES
        src/qml/Main.qml
)
