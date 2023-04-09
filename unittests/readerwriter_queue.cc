#include <gtest/gtest.h>
#include <thread>

#include "../readerwriter_queue.h"


TEST(NonBlockingQueueTest, TestInitialize)
{
    NonBlockingQueue<int> q;
    ASSERT_TRUE(q.is_empty());
    ASSERT_EQ(q.peek(), nullptr);
}

TEST(NonBlockingQueueTest, TestEnqueue)
{
    NonBlockingQueue<int> q;
    q.enqueue(5);
    ASSERT_EQ(*q.peek(), 5);
    ASSERT_FALSE(q.is_empty());
}

TEST(NonBlockingQueueTest, TestEnqueueMany)
{
    NonBlockingQueue<int> q;
    for (int i=0; i < 100; i++) {
        q.enqueue(i);
    }

    int item;
    for (int i=0; i < 100; i++) {
        q.dequeue(item);
        ASSERT_EQ(item, i);
    }
}

TEST(NonBlockingQueueTest, TestDequeue)
{
    NonBlockingQueue<int> q;
    int item;
    ASSERT_FALSE(q.dequeue(item));

    q.enqueue(5);
    ASSERT_TRUE(q.dequeue(item));
    ASSERT_EQ(item, 5);
    ASSERT_TRUE(q.is_empty());
}

TEST(NonBlockingQueueTest, TestExcessDequeue)
{
    NonBlockingQueue<int> q;
    for (int i=0; i < 100; i++) {
        q.enqueue(i);
    }

    int item;
    for (int i=0; i < 101; i++) { // try to dequeue when queue is empty
        q.dequeue(item);
    }

    ASSERT_TRUE(q.is_empty());
}

TEST(NonBlockingQueueTest, TestPop)
{
    NonBlockingQueue<int> q;
    for (int i=0; i < 100; i++) {
        q.enqueue(i);
    }

    int item;
    for (int i=0; i < 100; i++) {
        ASSERT_TRUE(q.pop());
    }

    ASSERT_TRUE(q.is_empty());
}

TEST(NonBlockingQueueTest, TestPeek)
{
    NonBlockingQueue<int> q;
    for (int i=0; i < 100; i++) {
        q.enqueue(i);
    }

    int item;
    for (int i=0; i < 100; i++) {
        if (q.peek() != nullptr)
        {
            ASSERT_EQ(*q.peek(), i);
        }
        q.dequeue(item);
    }
}

TEST(NonBlockingQueueTest, TestThreading)
{
    // TODO: figure out why test can't be ran concurrently
    NonBlockingQueue<int> q;
    std::thread writer([&]() {
        for (int i=0; i < 100; i++) {
            q.enqueue(i);
        }
    });
    writer.join();

    std::thread reader([&]() {
        int item;
        for (int i=0; i < 100; i++) {
            q.pop();
        }
    });
    reader.join();

    ASSERT_TRUE(q.is_empty());
}
