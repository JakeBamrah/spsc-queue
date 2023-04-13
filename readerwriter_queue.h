/* This is an implementaion for an unbounded SPSC lock-free queue based on the
   work of M.Maged and M.Scott (https://doi.org/10.1145/248052.248106).

   A lock-free single-producer, single-consumer queue. Queue head is always
   represented by a dummy-node where each node uses a modification counter that
   is incremented during CAS exchanges. This reduces the likelihood of the ABA
   problem during enqueue / dequeuing.

   HEAD(DUMMY)                          TAIL
   [value<T>, next*, counter] -> ... -> [value<T>, next*, counter] */

#include <atomic>
#include <cstdint>
#include <iostream>
#include <optional>


template<typename T>
class NodePointer;

template<typename T>
class Node
{
public:
    T value;
    NodePointer<T> next;

    Node()                              : next{NodePointer<T>()} {}
    Node(T val)                         : value{val}, next{NodePointer<T>()} {}
    Node(T val, NodePointer<T> next)    : value{val}, next{next} {}
};

template<typename T>
struct NodePointer
{
    Node<T>* node;
    size_t mod_counter;

    NodePointer(): node{nullptr}, mod_counter{0} {};
    NodePointer(Node<T>* n, size_t counter): node{n}, mod_counter{counter} {};

    bool operator==(const NodePointer<T>& other) const
    {
        return mod_counter == other.mod_counter && node == other.node;
    }

    bool operator!=(const NodePointer<T>& other) const
    {
        return !(this == other);
    };
};

template<typename T>
class NonBlockingQueue
{
public:
    NonBlockingQueue()
    {
        // initialize queue with head and tail as dummy node
        NodePointer<T> dummy = NodePointer<T>{new Node<T>(), 0};
        _head.store(dummy);
        _tail.store(dummy);
    }

    virtual ~NonBlockingQueue()
    {
        // queue should not be accessed once destructor has been called
        NodePointer<T> head = _head.load();
        while (head.node != nullptr)
        {
            auto old_head   = head;
            head            = head.node->next;
            delete old_head.node;
        }
    }

    bool is_empty()
    {
        return _tail.load(std::memory_order_acquire) == _head.load(std::memory_order_acquire);
    }

    /* PRODUCER METHOD: Enqueues a node (value) onto the back of the queue. */
    void enqueue(const T value)
    {
        // tail owned by consumer and producer so acquire where necessary
        NodePointer<T> tail = _tail.load(std::memory_order_acquire);
        Node<T>* new_node   = new Node<T>{value};
        while(!_tail.compare_exchange_weak(tail, NodePointer<T>{
                    new_node, tail.mod_counter + 1}))
        {
            // exchange failed because tail changed—reload tail and retry
            tail = _tail.load(std::memory_order_acquire);
        }
        tail.node->next = _tail.load(std::memory_order_acquire);
    }

     /* CONSUMER METHOD: Returns a pointer to the head *without* dequeuing it */
    T* peek()
    {
        NodePointer<T> head         = _head.load(std::memory_order_acquire);
        NodePointer<T> next_node_p  = head.node->next;
        if (head == _tail.load(std::memory_order_acquire) || next_node_p.node == nullptr) {
            return nullptr;
        }

        return &next_node_p.node->value;
    }

    bool try_dequeue(T& result)
    {
        while(true)
        {
            // only the consumer thread interacts with head so we can relax
            NodePointer<T> head         = _head.load(std::memory_order_relaxed);
            NodePointer<T> tail         = _tail.load(std::memory_order_acquire);
            NodePointer<T> next_node_p  = head.node->next;
            if (_head.load(std::memory_order_relaxed) == head)
            {
                // check if queue is empty or tail is lagging behind
                if (head == tail)
                {
                    if (next_node_p.node == nullptr) {
                        return false;
                    }
                    // try to update lagging tail to latest node
                    _tail.compare_exchange_weak(tail, NodePointer<T>{
                            next_node_p.node, tail.mod_counter + 1});
                }
                else // queue has valid nodes—attempt dequeue
                {
                    // read value before CAS—another dequeue might have occurred
                    // swing next_node to the new head / dummy node
                    T value = next_node_p.node->value;
                    if (_head.compare_exchange_weak(head, NodePointer<T>{
                            next_node_p.node, head.mod_counter + 1}))
                    {
                        // we are free to delete the old head :)
                        delete head.node;
                        result = std::move(value);
                        return true;
                    }
                }
            }
        }
    }

     /* CONSUMER METHOD: Dequeues and returns the value from the head node */
    bool dequeue(T& result)
    {
        return try_dequeue(result);
    }

     /* CONSUMER METHOD: Dequeues a node from the front of the queue (head) but
        does *not* return the value stored in the dequeued node. */
    bool pop()
    {
        // HACK: Needlessley create reference to avoid duplicating dequeue logic
        T result;
        return try_dequeue(result);
    }

private:
    std::atomic<NodePointer<T>> _head;
    std::atomic<NodePointer<T>> _tail;
};
