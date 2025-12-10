set(FETCHCONTENT_QUIET OFF)

set(ASSIMP_TAG v6.0.2) #https://github.com/assimp/assimp/tags
set(FREETYPE_TAG VER-2-14-1) #https://github.com/freetype/freetype/tags
set(FMT_TAG 12.1.0) #https://github.com/fmtlib/fmt/tags
set(GLFW_TAG 3.4) #https://github.com/glfw/glfw/tags
set(GLM_TAG 1.0.2) #https://github.com/g-truc/glm/tags
set(KTX_TAG v4.4.2) #https://github.com/KhronosGroup/KTX-Software/tags
set(LUA_TAG v5.4.8) #https://github.com/lua/lua/tags
set(MINIAUDIO_TAG  0.11.23) #https://github.com/mackron/miniaudio/tags
set(NH_JSON_TAG v3.12.0) #https://github.com/nlohmann/json/tags
set(STB_TAG master) #https://github.com/nothings/stb
set(SQLITE_TAG 3510000) #https://www.sqlite.org/download.html https://github.com/sqlite/sqlite/tags
set(VOLK_TAG vulkan-sdk-1.4.335.0) #https://github.com/zeux/volk/tags
#set(WxWidget_TAG v3.3.0)
set(FETCH_GIT_SHALLOW TRUE)
set(FETCH_GIT_PROGRESS TRUE)
set(FETCH_UPDATE_DISCONNECTED TRUE)

include(FetchContent)

include(deps/FetchAssimp)
include(deps/FetchFreeType)
include(deps/FetchGLFW)
include(deps/FetchGLM)
include(deps/FetchFMT)
include(deps/FetchKTX)
include(deps/FetchLua)
include(deps/FetchMiniAudio)
include(deps/FetchNlohmannJson)
include(deps/FetchSTB)
include(deps/FetchSQlite)
include(deps/FetchTclTk)
include(deps/FetchVolk)
#include(deps/FetchWxWidgets)