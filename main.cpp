#include <iostream>
#include <queue>
#include <utility>
#include <type_traits>

#include <string>
#include <type_traits>
#include <thread>
#include <mutex>
#include "error.hpp"



namespace type_traits {
template <class T> struct is_serializable : std::false_type {};

template <class T>
    requires requires(T x) {
        { std::declval<std::ostream&>() << x };
    }
struct is_serializable<T> : std::true_type {};

template <class T> struct is_serializable_nonptr
    : std::bool_constant<!std::is_pointer_v<T> && is_serializable<T>::value> {};

}; // namespace type_traits


class ChanError : public std::exception {
    public:
        ChanError(const char* msg) : std::exception() {}

        const char* what() const noexcept override { return msg.c_str(); }

    private:
        std::string msg;
};



template <class T>
    requires type_traits::is_serializable_nonptr<T>::value
class Channel;


template <class T> struct larrow {

        template <class U = T>
        requires(!std::is_signed_v<T> || !std::is_arithmetic_v<T>)
        larrow(const T& x)
            : data(std::move(x)) {
//            std::println("con1: {}", data);
        }

        template <class U = T>
            requires std::is_signed<U>::value && std::is_arithmetic<U>::value
        larrow(const U& x) : data(-x) {
//            std::println("con2: {}", data);
        }

        auto get_data() const { return data; }
    private:
        T data;
};

template <class U>
    requires type_traits::is_serializable_nonptr<U>::value
larrow<U> operator-(U&& x) {
    return larrow<U>(std::forward<U>(x));
}

template <class T> struct larrow_out {

        template <class U = T>
        larrow_out(const T& x)
            : data(std::move(x)) {
            //            std::println("con1: {}", data);
        }

        auto get_data() const { return data; }
    private:
        T data;
};

template <class U>
    requires type_traits::is_serializable_nonptr<U>::value && std::is_signed_v<U>
larrow_out<U> operator-(U&& x) {
    return larrow_out<U>(-x);
}



template <class T>
    requires type_traits::is_serializable_nonptr<T>::value
class Channel {

    public:
        static Channel<T> make_chan() { return Channel<T>(); }

        template <class U>
            requires type_traits::is_serializable_nonptr<U>::value
        friend larrow_out<U> operator-(Channel<U>& chan);

        friend void operator<(Channel<T>& lhs, const larrow<T>& rhs) {
            if(lhs.is_closed) {
                throw std::runtime_error("Channel is closed");
            }
            std::lock_guard<std::mutex> lock(lhs.mtx);
            lhs.data.push(rhs.get_data());
            lhs.cv.notify_one();
        }

        inline constexpr void close() {
            std::lock_guard<std::mutex> lock(mtx);
            is_closed = true;
        }

    private:
        std::condition_variable cv;
        std::queue<T> data;
        std::mutex mtx;
        std::atomic_bool is_closed{false};
};


template <class U>
    requires type_traits::is_serializable_nonptr<U>::value
larrow_out<U> operator-(Channel<U>& chan) {
    if(chan.is_closed) {
        throw std::runtime_error("Channel is closed");
    }
    std::unique_lock<std::mutex> lock(chan.mtx);
    chan.cv.wait(lock, [&chan] { return !chan.data.empty(); });
    auto data = chan.data.front();
    chan.data.pop();
    return larrow_out<U>(data);
}

template <class T> void drain(Channel<T>& chan) {
    std::lock_guard<std::mutex> lock(chan.mtx);
    while (!chan.data.empty()) {
        std::println("{}", chan.data.front());
        chan.data.pop();
    }
}

template <class T>
    requires type_traits::is_serializable_nonptr<T>::value
T operator<(T& lhs, larrow_out<T> rhs) {
    lhs = rhs.get_data();
    return lhs;
}

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


class WaitGroup {
    public:
        void add(int n = 1) { count += n; }
        void done() { --count; }
        void wait() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return count == 0; });
        }
    private:
        std::condition_variable cv;
        std::mutex mtx;
        int count = 0;
};

void func(int a, int b) {
    std::print("a: {}, b: {}\n", a, b);
}


template <typename F>
struct Defer {
        F f;
        Defer(F f) : f(f) {}
        ~Defer() { f(); }
};

template <typename F>
Defer<F> defer_func(F f) {
    return Defer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})


int main() {

    defer(std::print("deferred\n"));


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
