module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

export module Engine.Core.GLM;

export namespace glm
{
  using glm::ivec2;
  using glm::vec2;
  using glm::vec3;
  using glm::vec4;
  using glm::mat3;
  using glm::mat4;
  using glm::quat;
  using glm::fvec2;
  using glm::dquat;

  using glm::translate;
  using glm::rotate;
  using glm::scale;
  using glm::perspective;
  using glm::lookAt;
  using glm::inverse;
  using glm::transpose;
  using glm::normalize;
  using glm::cross;
  using glm::dot;
  using glm::mix;
  using glm::radians;
  using glm::degrees;
  using glm::mat4_cast;
  using glm::value_ptr;
  using glm::decompose;
  using glm::length;
  using glm::distance;
  using glm::clamp;
  using glm::slerp;
  using glm::cos;
  using glm::sin;
  using glm::angleAxis;
  using glm::acos;
  using glm::asin;
  using glm::atan;
  using glm::eulerAngles;
  using glm::two_pi;
  using glm::ortho;

  using ::glm::operator+;
  using ::glm::operator-;
  using ::glm::operator*;
  using ::glm::operator/;
  using ::glm::operator==;
  using ::glm::operator!=;

  using namespace ::glm;
}
