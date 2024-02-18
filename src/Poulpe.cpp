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
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wmissing-prototypes"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#pragma clang diagnostic pop

int main(int argc, char** argv)
{
    std::cout << argv[0] << " Version " << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR << std::endl;

    bool serverMode{ false };
    std::string port{ "9371" };

    for (int i = 0; i < argc; ++i) {
      std::string argument = argv[i];
      if ("--server" == argument || "-S" == argument) {
        serverMode = true;
      } else if (argument.find("--port") != std::string::npos) {
        std::size_t pos = argument.find("=");
        if (pos == std::string::npos) {
          PLP_WARN("bad argument {}", argument);
        } else {
          port = argument.substr(++pos, argument.size());
        }
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
