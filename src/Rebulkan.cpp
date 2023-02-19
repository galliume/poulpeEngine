#include "RebulkanConfig.h"
#include "Rebulk/Application.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
      std::cout << argv[0] << " Version " << Rebulkan_VERSION_MAJOR << "."
              << Rebulkan_VERSION_MINOR << std::endl;
    }

    std::unique_ptr<Rbk::Application>app = std::make_unique<Rbk::Application>();
    app->Init();
    app->Run();

    return 0;
}