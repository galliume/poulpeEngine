export module Poulpe.Core:joinThreads;

import <vector>;
import <thread>;

export class joinThreads
{
public:
    explicit joinThreads(std::vector<std::thread>& threads) : _Threads(threads) {};
    ~joinThreads() {
        for (unsigned long i{ 0 }; i < _Threads.size(); ++i) {
            if (_Threads[i].joinable()) {
                _Threads[i].join();
            }
        }
    }
private:
    std::vector<std::thread>& _Threads;
};
