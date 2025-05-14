module;
#define NOMINMAX

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "PoulpeEngineConfig.h"

import Application;

namespace Poulpe
{
  #define STB_IMAGE_IMPLEMENTATION
  #include <stb_image.h>

  int main(int argc, char** argv)
  {
      std::cout << argv[0] << " Version " << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR << std::endl;

      bool server_mode{ true };
      std::string port{ "9371" };

      for (int i { 0 }; i < argc; ++i) {
        std::string argument = argv[i];
        if ("--server" == argument || "-S" == argument) {
          server_mode = true;
        } else if (argument.find("--port") != std::string::npos) {
          std::size_t pos = argument.find("=");
          if (pos == std::string::npos) {
            Logger::warn("bad argument {}", argument);
          } else {
            port = argument.substr(++pos, argument.size());
          }
        }
      }
      std::unique_ptr<Application> app = std::make_unique<Application>();
      app->init();

      Logger::trace("server_mode {}", server_mode);

      if (server_mode) {
        Logger::trace("port {}", port);
        app->startServer(port);
      }

      app->run();

      return 0;
  }
}
