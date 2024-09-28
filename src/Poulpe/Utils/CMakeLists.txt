set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/IDHelper.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/LuaScript.hpp
			 ${CMAKE_CURRENT_SOURCE_DIR}/ScopedTimer.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})