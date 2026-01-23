export module Engine.Core.Constants;

import std;

namespace Poulpe
{
  //default textures const, needs a real assets management (unique id etc.)
  export inline constexpr std::string PLP_EMPTY{ "_plp_empty" };
  export inline constexpr std::string PLP_ERROR{ "_plp_error" };
  export inline constexpr std::string PLP_SAND{ "sand" };
  export inline constexpr std::string PLP_GRASS{ "grass" };
  export inline constexpr std::string PLP_GROUND{ "ground" };
  export inline constexpr std::string PLP_LOW_NOISE{ "low_noise" };
  export inline constexpr std::string PLP_HI_NOISE { "hi_noise" };
  export inline constexpr std::string PLP_SNOW{ "snow" };
  export inline constexpr std::string PLP_WATER_NORMAL_1{ "_water_normal" };
  export inline constexpr std::string PLP_WATER_NORMAL_2{ "_water_normal2" };

  export enum PLP_ENV_OPTIONS : std::uint32_t {
    HAS_FOG = 1 << 0,
    HAS_IRRADIANCE = 1 << 1
  };

  export enum PLP_MESH_OPTIONS : std::uint32_t {
    HAS_BASE_COLOR = 1 << 0,
    HAS_SPECULAR = 1 << 1,
    HAS_NORMAL = 1 << 2,
    HAS_ALPHA = 1 << 3,
    HAS_MR = 1 << 4,
    HAS_EMISSIVE = 1 << 5,
    HAS_AO = 1 << 6,
    HAS_TRANSMISSION = 1 << 7
  };

  export enum class TextureWrapMode {
    WRAP,
    CLAMP_TO_EDGE,
    MIRROR_REPEAT
  };

  export enum class AnimInterpolation {
    STEP,
    LINEAR,
    SPHERICAL_LINEAR,
    CUBIC_SPLINE
  };

  export enum class SocketStatus {
    NOT_CONNECTED,
    CONNECTED
  };

  export enum class ServerStatus {
    NOT_RUNNING,
    RUNNING
  };
}
