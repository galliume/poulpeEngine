module;

#include <vector>
#include <thread>

export module Poulpe.Core.JoinThreads;

namespace Poulpe
{
  export class JoinThreads
  {
  public:
    explicit JoinThreads(std::vector<std::thread>& threads) : _threads(threads) {}
    ~JoinThreads() {
      for (unsigned long i{ 0 }; i < _threads.size(); ++i) {
        if (_threads[i].joinable()) {
          _threads[i].join();
        }
      }
    }
  private:
    std::vector<std::thread>& _threads;
  };
}
