#include <gtest/gtest.h>
#include <thread>

#include "../readerwriter_queue.h"


TEST(QueueTest, TestInitialize)
{
    NonBlockingQueue<int> q;
    ASSERT_TRUE(q.is_empty());
    ASSERT_EQ(q.peek(), nullptr);
}

TEST(QueueTest, TestEnqueue)
{
    NonBlockingQueue<int> q;
    q.enqueue(5);
    ASSERT_EQ(*q.peek(), 5);
    ASSERT_FALSE(q.is_empty());
}

TEST(QueueTest, TestEnqueueMany)
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

TEST(QueueTest, TestDequeue)
{
    NonBlockingQueue<int> q;
    int item;
    ASSERT_FALSE(q.dequeue(item));

    q.enqueue(5);
    ASSERT_TRUE(q.dequeue(item));
    ASSERT_EQ(item, 5);
    ASSERT_TRUE(q.is_empty());
}

TEST(QueueTest, TestPop)
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

TEST(QueueTest, TestPeek)
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

TEST(QueueTest, TestThreading)
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
