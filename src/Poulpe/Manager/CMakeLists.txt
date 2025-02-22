set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/AudioManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/AudioManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ConfigManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ConfigManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/DestroyManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/DestroyManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/EntityManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/EntityManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/InputManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/InputManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/RenderManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/RenderManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ShaderManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ShaderManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/TextureManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/TextureManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ComponentManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ComponentManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/SystemManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/SystemManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/LightManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/LightManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/NetworkManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/NetworkManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/APIManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/APIManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/FontManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/FontManager.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})
