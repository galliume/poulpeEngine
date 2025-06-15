module;

#include <latch>
#include <ranges>
#include <vector>
#include <variant>

export module Engine.Managers.APIManager;

namespace Poulpe
{
  export class APIManager
  {
  public:
    APIManager() = default;
    ~APIManager()  = default;

    void received(std::string const& message);

  private:
    void updateSkybox(std::vector<std::string> const & params);
  };
}
