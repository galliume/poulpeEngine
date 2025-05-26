#include "PoulpeEngineConfig.h"
#include <iostream>
#include <memory>

import Poulpe.Application;
import Poulpe.Core.Logger;

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
        Poulpe::Logger::warn("bad argument {}", argument);
      } else {
        port = argument.substr(++pos, argument.size());
      }
    }
  }
  std::unique_ptr<Poulpe::Application> app = std::make_unique<Poulpe::Application>();
  app->init();

  Poulpe::Logger::trace("server_mode {}", server_mode);

  if (server_mode) {
    Poulpe::Logger::trace("port {}", port);
    app->startServer(port);
  }

  app->run();

  return 0;
}
