add_executable (PaperLuaTests PaperLuaTests.cpp)
target_link_libraries(PaperLuaTests ${PAPERLUADEPS})
add_custom_target(check COMMAND PaperLuaTests)