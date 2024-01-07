#include "PoulpeEngineConfig.h"

#include "Poulpe/Application.hpp"

#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
#pragma clang diagnostic ignored "-Wreserved-macro-identifier"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wcast-align"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"

#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma clang diagnostic pop

int main(int argc, char** argv)
{
    if (argc < 2) {
      std::cout << argv[0] << " Version " << PoulpeEngine_VERSION_MAJOR << "."
              << PoulpeEngine_VERSION_MINOR << std::endl;
    }

    std::unique_ptr<Poulpe::Application>app = std::make_unique<Poulpe::Application>();
    app->init();
    app->run();

    return 0;
}
