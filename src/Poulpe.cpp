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
    std::cout << argv[0] << " Version " << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR << std::endl;

    bool serverMode{ false };
    std::string port{ "8289" };

    //@todo clean cmd line parser
    for (int i = 1; i < argc; ++i) {
      switch (i) {
      case 1:
        serverMode = true;
        break;
      case 2:
        port = argv[i];
        break;
      }
    }
    std::unique_ptr<Poulpe::Application> app = std::make_unique<Poulpe::Application>();
    app->init();

    PLP_TRACE("serverMode {}", serverMode);

    if (serverMode) {
      PLP_TRACE("port {}", port);
      app->startServer(port);
    }

    app->run();

    return 0;
}
