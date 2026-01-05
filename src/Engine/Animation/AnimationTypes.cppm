export module Engine.Animation.AnimationTypes;

import Engine.Core.MeshTypes;

namespace Poulpe {
  export struct AnimationInfo
  {
    double const delta_time{0.0};
    Data* const data {};
    bool looping { true };
  };
}
