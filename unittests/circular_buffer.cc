#include <gtest/gtest.h>
#include <thread>

#include "../circular_buffer.h"


TEST(CircularBufferTest, TestInitialize)
{
    CircularBuffer<int, 100> q;
    ASSERT_TRUE(q.is_empty());
}

TEST(CircularBufferTest, TestEnqueue)
{
    CircularBuffer<int, 100> q;
    q.enqueue(5);
    ASSERT_EQ(*q.peek(), 5);
    ASSERT_FALSE(q.is_empty());
}

TEST(CircularBufferTest, TestEnqueueMany)
{
    CircularBuffer<int, 100> q;
    for (int i=0; i < 100; i++) {
        q.enqueue(i);
    }

    int item;
    for (int i=0; i < 100; i++) {
        q.dequeue(item);
        ASSERT_EQ(item, i);
    }
}

TEST(CircularBufferTest, TestExcessEnqueue)
{
    CircularBuffer<int, 100> q;
    for (int i=0; i < 101; i++) {
        q.enqueue(i);
    }

    ASSERT_TRUE(q.is_full());
}

TEST(CircularBufferTest, TestDequeue)
{
    CircularBuffer<int, 100> q;
    int item;
    ASSERT_FALSE(q.dequeue(item));

    q.enqueue(5);
    ASSERT_TRUE(q.dequeue(item));
    ASSERT_EQ(item, 5);
    ASSERT_TRUE(q.is_empty());
}

TEST(CircularBufferTest, TestExcessDequeue)
{
    CircularBuffer<int, 100> q;
    for (int i=0; i < 100; i++) {
        q.enqueue(i);
    }

    int item;
    for (int i=0; i < 101; i++) { // try to dequeue when queue is empty
        q.dequeue(item);
    }

    ASSERT_TRUE(q.is_empty());
}

TEST(CircularBufferTest, TestPeek)
{
    CircularBuffer<int, 100> q;
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

TEST(CircularBufferTest, TestThreading)
{
    CircularBuffer<int, 100> q;
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
