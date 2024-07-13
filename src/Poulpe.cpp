#include "PoulpeEngineConfig.h"

#include "Poulpe/Application.hpp"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


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
