#include <gtest/gtest.h>
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