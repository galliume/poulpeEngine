export module Engine.Core.LockFreeStack;

import std;

namespace Poulpe
{
  export template<typename T>
  class LockFreeStack
  {
  public:
    void push(T data)
    {
      node* const newNode = new node(data);
      newNode->next = head.load();

      while (!head.compare_exchange_weak(newNode->next, newNode));
    }

    std::unique_ptr<T> pop()
    {
      ++threadsInPop;

      node* oldHead = head.load();

      while (oldHead && !head.compare_exchange_weak(oldHead, oldHead->next));

      std::unique_ptr<T> res;

      if (oldHead) {
        res.swap(oldHead->data);
      }

      tryReclaim(oldHead);

      return res;
    }

  private:
    struct node
    {
      std::unique_ptr<T> data;
      node* next;

      node(T _data)
        : data(std::make_unique<T>(std::move(_data)))
      {

      }
    };
    std::atomic<node*> head;
    std::atomic<node*> toBeDeleted;
    std::atomic<unsigned> threadsInPop{ 0 };

    static void deleteNodes(node* nodes)
    {
      while (nodes) {
        node* next = nodes->next;
        delete nodes;
        nodes = next;
      }
    }
    void tryReclaim(node* oldHead)
    {
      if (threadsInPop == 1) {
        node* nodesToDelete = toBeDeleted.exchange(nullptr);

        if (!--threadsInPop) {
          deleteNodes(nodesToDelete);
        } else if (nodesToDelete) {
          chainPendingNodes(nodesToDelete);
        }
        delete oldHead;
      } else {
        chainPendingNode(oldHead);
        --threadsInPop;
      }
    }

    void chainPendingNodes(node* nodes)
    {
      node* last = nodes;

      while (node* const next = last->next) {
        last = next;
      }

      chainPendingNodes(nodes, last);
    }

    void chainPendingNodes(node* first, node* last)
    {
      if (!first || !last) return;

      last->next = toBeDeleted;

      while (!toBeDeleted.compare_exchange_weak(last->next, first));
    }

    void chainPendingNode(node* n)
    {
      chainPendingNodes(n, n);
    }
  };
}
