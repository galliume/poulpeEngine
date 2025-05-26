module;
#include "PoulpeEngineConfig.h"

#include <iostream>

export module Poulpe;

import Poulpe.Application;

int main(int argc, char** argv)
{
  std::cout << argv[0] << " Version " << PoulpeEngine_VERSION_MAJOR << "." << PoulpeEngine_VERSION_MINOR << std::endl;

  bool serverMode{ false };
  std::string port{ "9371" };

  for (int i { 0 }; i < argc; ++i) {
    std::string argument = argv[i];
    if ("--server" == argument || "-S" == argument) {
      serverMode = true;
    } else if (argument.find("--port") != std::string::npos) {
      std::size_t pos = argument.find("=");
      if (pos == std::string::npos) {
        Logger::warn("bad argument {}", argument);
      } else {
        port = argument.substr(++pos, argument.size());
      }
    }
  }
  std::unique_ptr<Poulpe::Application> app = std::make_unique<Poulpe::Application>();
  app->init();

   Logger::trace("serverMode {}", serverMode);

   if (serverMode) {
    Logger::trace("port {}", port);
    app->startServer(port);
  }

   app->run();

   return 0;
}
