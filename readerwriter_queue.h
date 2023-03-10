#include <atomic>
#include <cstdint>
#include <iostream>
#include <optional>


template<typename T>
class Node {
public:
    T value;
    Node<T>* next;
    int modification_counter{0};

    Node() :next{nullptr} {}

    Node(T val)
        :value{val},
        next{nullptr}
    {}

    Node(T val, int counter)
        :value{val},
        modification_counter{counter}
    {}

    Node(T val, Node<T>* next, int counter)
        :value{val},
        next{next},
        modification_counter{counter}
    {}
};


/*
 * This is an implementaion for a SPSC lock-free
 * queue based on the work of M.Maged and M.Scott.
 * https://doi.org/10.1145/248052.248106
 *
 * A lock-free single-producer, single-consumer queue. Queue head is always
 * represented by a dummy-node where each node uses a modification counter that
 * is incremented during CAS exchanges. This reduces the likelihood of the ABA
 * problem during enqueue / dequeuing.
 *
 * HEAD(DUMMY)                          TAIL
 * {value<T>, next*, counter} -> ... -> {value<T>, next*, counter}
 */
template<typename T>
class NonBlockingQueue {
public:
    NonBlockingQueue()
    {
        // initialize queue with head and tail as dummy node
        Node<T>* dummy = new Node<T>;
        head.store(dummy);
        tail.store(dummy);
    };

    ~NonBlockingQueue()
    {
        // queue should not be accessed once destructor has been called
        auto current_head = head.load();
        while (current_head->next != nullptr)
        {
            auto old_head = current_head;
            current_head = current_head->next;
            delete old_head;
        }
    };

    bool is_empty()
    {
        return tail.load() == head.load();
    };

    /*
     * Producer method: Enqueues a value as a node onto the back of the queue.
     */
    void enqueue(const T value)
    {
        Node<T>* current_tail = tail.load();
        Node<T>* p = new Node<T>{value, current_tail->modification_counter + 1};
        while(!tail.compare_exchange_weak(current_tail, p))
        {
            // exchange failed because tail changed???reload tail and retry
            current_tail = tail.load();
            p->modification_counter = current_tail->modification_counter + 1;
        }

        // TODO: Race? What if current_tail is consumed before assigning tail.next
        current_tail->next = p;
    };

    /*
     * Consumer method: Returns a pointer to the element belonging to the queue
     * head-node without dequeueing it???returns a nullptr if queue is empty.
     */
    T* peek()
    {
        Node<T>* current_head = head.load();
        Node<T>* next_node = current_head->next;
        if (current_head == tail.load() || next_node == nullptr)
            return nullptr;

        return &next_node->value;
    };

    bool try_dequeue(T& result)
    {
        while(true)
        {
            Node<T>* current_head = head.load();
            Node<T>* current_tail = tail.load();
            Node<T>* next_node = current_head->next;
            if (head.load() == current_head)
            {
                // check if queue is empty or tail is lagging behind
                if (current_head == current_tail)
                {
                    if (next_node == nullptr) {
                        return false;
                    }
                    // try to update lagging tail to latest node
                    tail.compare_exchange_weak(current_tail, new Node<T>{
                                next_node->value,
                                next_node->next,
                                current_tail->modification_counter + 1});
                }
                // queue has valid nodes???attempt dequeue
                else
                {
                    // read value before CAS???another dequeue might have occurred
                    // swing next_node to the new head / dummy node
                    T value = next_node->value;
                    next_node->modification_counter = current_head->modification_counter + 1; // TODO: modification_counter should be incremented in CAS?
                    if (head.compare_exchange_weak(current_head, next_node))
                    {
                        // we are free to delete the old head :)
                        result = value;
                        delete current_head;
                        return true;
                    }
                }
            }
        }
    };

    /*
     * Consumer method: Dequeues a node from the front of the queue (head) and
     * returns the value stored in the dequeued node.
     */
    bool dequeue(T& result)
    {
        return try_dequeue(result);
    };

    /*
     * Consumer method: Dequeues a node from the front of the queue (head) but
     * does *not* return the value stored in the dequeued node.
     */
    bool pop()
    {
        // HACK: Needlessley create reference to avoid duplicating dequeue logic
        T result;
        return try_dequeue(result);
    };

private:
    std::atomic<Node<T>*> head;
    std::atomic<Node<T>*> tail;
};
