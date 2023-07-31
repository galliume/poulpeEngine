set(SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/AudioManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/AudioManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ConfigManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ConfigManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/DestroyManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/DestroyManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/EntityManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/EntityManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/IAudioManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/IConfigManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/IEntityManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/IInputManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/InputManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/InputManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/IRenderManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/IShaderManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ISpriteAnimationManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ITextureManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/RenderManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/RenderManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ShaderManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/ShaderManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/SpriteAnimationManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/SpriteAnimationManager.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/TextureManager.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/TextureManager.hpp)

target_sources(PoulpeEngine PRIVATE ${SRC_LIST})