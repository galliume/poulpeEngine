#include "RebulkanConfig.h"
#include "Rebulk/Application.hpp"

int main(int argc, char** argv)
{
    if (argc < 2) {
      std::cout << argv[0] << " Version " << Rebulkan_VERSION_MAJOR << "."
              << Rebulkan_VERSION_MINOR << std::endl;
    }

   std::unique_ptr<Rbk::Application>app = std::make_unique<Rbk::Application>();
  
    return 0;
}