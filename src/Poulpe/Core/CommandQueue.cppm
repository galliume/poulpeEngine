export module Poulpe.Core:CommandQueue;

import Command;

import <memory>;
import <mutex>;
import <queue>;

export class CommandQueue
{
public:
    CommandQueue();
    ~CommandQueue() = default;

    void add(Command& cmd);
    void execPostRequest();
    void execPreRequest();

private:
    mutable std::mutex _Mutex;

    std::queue<std::shared_ptr<Command>> _PostCmdQueue;
    std::queue<std::shared_ptr<Command>> _PreCmdQueue;
};
