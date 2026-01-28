module;

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

export module Engine.Core.StbImage;

export
{
  using ::stbi_uc;
  using ::stbi_load;
  using ::stbi_image_free;
  using ::stbi_set_flip_vertically_on_load;
  
  using ::STBI_default;
  using ::STBI_grey;
  using ::STBI_grey_alpha;
  using ::STBI_rgb;
  using ::STBI_rgb_alpha;
}
