#include "PoulpeEngineConfig.h"
#include "Poulpe/Application.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
      std::cout << argv[0] << " Version " << PoulpeEngine_VERSION_MAJOR << "."
              << PoulpeEngine_VERSION_MINOR << std::endl;
    }

    std::unique_ptr<Poulpe::Application>app = std::make_unique<Poulpe::Application>();
    app->Init();
    app->Run();

    return 0;
}