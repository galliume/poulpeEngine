export module Poulpe.Animation.AnimationTypes;

import Poulpe.Core.MeshTypes;

namespace Poulpe {
  export struct AnimationInfo
  {
    double const delta_time{0.0};
    Data* const data {};
  };
}
