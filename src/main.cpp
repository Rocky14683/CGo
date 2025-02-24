#include <iostream>
#include <queue>
#include <utility>
#include <type_traits>

#include <string>
#include <type_traits>
#include <thread>
#include <mutex>
#include "error.hpp"
#include "defer.hpp"
#include "waitGroup.hpp"
#include "channel.hpp"




class go {
    public:
        friend class WaitGroup;
        template <class F> go(F&& f) { std::thread(std::forward<F>(f)).detach(); }

        template <class F, class...Args> go(F&& f, Args...args) { std::thread(std::forward<F>(f), args...).detach(); }

        ~go() {
            cv.notify_all();
        }
    private:
        std::condition_variable cv;
};




void func(int a, int b) {
    std::print("a: {}, b: {}\n", a, b);
}





int main() {

    defer(std::print("deferred\n"));
    defer(std::print("deferred2\n"));

    auto chanInt = Channel<int>::make_chan();
    chanInt <- -42;
    chanInt <- 43;

    auto chanUint = Channel<unsigned int>::make_chan();
    chanUint <- 12;
    chanUint <- 69;
    auto chanString = Channel<std::string>::make_chan();
    chanString <- std::string("Hello, World!");
    WaitGroup wg;

    go([&] { chanString <- std::string("not hi"); });
    wg.add(1);

    go(func, 1, 2);
    wg.add(1);
    go([&] {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    });
    wg.add(1);


    int c;
    c <- chanInt;
    std::print("{}\n", c);
    c <- chanInt;
    std::print("{}\n", c);

    unsigned int u;
    u <- chanUint;
    std::print("{}\n", u);
    u <- chanUint;
    std::print("{}\n", u);


    using namespace std::chrono_literals;
    std::this_thread::sleep_for(200ms);
    std::string i;
    i <- chanString;
    std::print("{}\n", i);
    i <- chanString;
    std::print("{}\n", i);
//    wg.wait();
    printf("done\n");
    return 0;
}
