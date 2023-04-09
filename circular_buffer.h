/* A simple circular buffer that uses sequentially consistent reads. */

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
        const auto current_tail = _tail.load();
        auto next_tail          = increment(current_tail);
        if (next_tail == _head.load())
            return false; // full

        _array[current_tail] = value;
        _tail.store(next_tail);
        return true;
    }

    /* CONSUMER MEHOD: Updates head index *after* removing element */
    bool dequeue(NodeType& value)
    {
        const auto current_head = _head.load();
        if (current_head == _tail.load())
            return false; // empty

        value = _array[current_head];
        _head.store(increment(current_head));
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
