/* A simple circular buffer that uses refined-memory ordered reads / writes.
   Producer thread will only update tail and Consumer will only update head. */

#include <atomic>
#include <cstddef>


template<typename NodeType, size_t Size>
class CircularBuffer {
public:
    enum { Capacity = Size + 1 };

    CircularBuffer(): _tail{0}, _head{0} {}
    virtual ~CircularBuffer() {}

    /* PRODUCER METHOD: Updates tail index *after* placing element into queue */
    bool enqueue(const NodeType& value)
    {
        // use relaxed here because only one producer thread will modify
        // tail-this means we are sure to have the latest value for tail
        const auto current_tail = _tail.load(std::memory_order_relaxed);
        auto next_tail          = increment(current_tail);
        if (next_tail == _head.load(std::memory_order_acquire))
            return false; // full

        _array[current_tail] = value;
        _tail.store(next_tail, std::memory_order_release);
        return true;
    }

    /* CONSUMER MEHOD: Updates head index *after* removing element */
    bool dequeue(NodeType& value)
    {
        const auto current_head = _head.load(std::memory_order_relaxed);
        if (current_head == _tail.load(std::memory_order_acquire))
            return false; // empty

        value = _array[current_head];
        _head.store(increment(current_head), std::memory_order_release);
        return true;
    }

    /* CONSUMER MEHOD: Dequeues node without returning a value */
    bool pop()
    {
        NodeType value;
        return dequeue(value);
    }

     /* CONSUMER MEHOD: Returns a pointer to head *without* dequeueing it */
    NodeType* peek()
    {
        auto current_head   = _head.load();
        if (is_empty())
            return nullptr;

        return &_array[current_head];
    }

    /* Snapshot of empty and full queue status */
    bool is_empty() { return _head.load() == _tail.load(); }
    bool is_full()
    {
        auto next_tail = increment(_tail.load());
        return next_tail == _head.load();
    }

private:
    size_t increment(size_t idx) const { return (idx + 1) % Capacity; }

    NodeType _array[Capacity];
    std::atomic<size_t> _head;
    std::atomic<size_t> _tail;
};
