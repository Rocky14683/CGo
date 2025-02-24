#include <gtest/gtest.h>
#include <atomic>
#include "defer.hpp"
#include "channel.hpp"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}

TEST(defer_test, BasicAssertions) {
    int x = 0;
    int y = 256;
    {
        SCOPED_TRACE("Inside the scope");
        defer(x = 1);
        defer(y++);
        EXPECT_EQ(x, 0);
        EXPECT_EQ(y, 256);
    } // The defer statements should execute here
    EXPECT_EQ(x, 1);
    EXPECT_EQ(y, 257);
}

TEST(channel_test, BasicAssertions) {
    auto chanInt = Channel<int>::make_chan();
    chanInt <- -42;

    auto chanUint = Channel<unsigned int>::make_chan();
    chanUint <- 12;

    auto chanString = Channel<std::string>::make_chan();
    chanString <- std::string("Hello, World!");
    int i;
    i <- chanInt;
    EXPECT_EQ(i, -42);

    unsigned int u;
    u <- chanUint;

    EXPECT_EQ(u, 12);

    std::string s;
    s <- chanString;
    EXPECT_EQ(s, "Hello, World!");
}

TEST(buffered_channel_test, BasicAssertions) {
    auto chanUint = Channel<unsigned int>::make_chan(2);
    chanUint <- 12;
    chanUint <- 24;

    unsigned int u;
    u < -chanUint;
    EXPECT_EQ(u, 12);

    u < -chanUint;
    EXPECT_EQ(u, 24);

    std::atomic_bool done {false};

    auto chanInt = Channel<int>::make_chan(2);

    std::thread([&]() {
        chanInt <- 11;
        chanInt <- 22;
        chanInt <- 33;
        done.store(true);
    }).detach();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_FALSE(done.load()) << "Channel overflow should block the thread";
}


TEST(for_each_channel_test, BasicAssertions) {
    auto chanInt = Channel<int>::make_chan();
    chanInt <- 1;
    chanInt <- 2;
    chanInt <- 3;
    chanInt <- 1;
    chanInt <- 2;
    chanInt <- 3;

    int sum = 0;
    for (auto i : chanInt) {
        sum += i;
    }

    EXPECT_EQ(sum, 12);
}