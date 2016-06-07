cmake_minimum_required (VERSION 2.8.11)
project (PaperLua)
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-exceptions")

find_package(Lua 5.2 REQUIRED)
include_directories (${CMAKE_CURRENT_SOURCE_DIR} /usr/local/include  ${LUA_INCLUDE_DIR})
link_directories(/usr/local/lib)

set (PAPERLUADEPS Stick Brick Paper pthread ${LUA_LIBRARIES})

set (PAPERLUAINC
PaperLua/PaperLua.hpp
)

install (FILES ${PAPERLUAINC} DESTINATION /usr/local/include/PaperLua)
add_subdirectory (Tests)
