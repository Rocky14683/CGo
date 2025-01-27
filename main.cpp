#include <iostream>
#include <queue>
#include <utility>
#include <type_traits>

#include <string>
#include <type_traits>
#include <thread>
#include <mutex>

namespace type_traits {
// SFINAE stuffs
template <class T> struct is_serializable : std::false_type {};

template <class T>
    requires requires(T x) {
        { std::declval<std::ostream&>() << x };
    }
struct is_serializable<T> : std::true_type {};

template <class T> struct is_serializable_nonptr
    : std::bool_constant<!std::is_pointer_v<T> && is_serializable<T>::value> {};
}; // namespace type_traits

template <class T> struct larrow {
        T data;

        larrow(T x) : data(std::move(x)) {}
};

template <class U>
    requires type_traits::is_serializable_nonptr<U>::value
larrow<U> operator-(U&& x) {
    return larrow<U>(std::forward<U>(x));
}

template <class T>
    requires type_traits::is_serializable_nonptr<T>::value
class Channel;

template <class U>
    requires type_traits::is_serializable_nonptr<U>::value
larrow<U> operator-(Channel<U>& chan) {
    std::lock_guard<std::mutex> lock(chan.mtx);
    auto data = chan.data.front();
    chan.data.pop();
    return larrow<U>(data);
}

template <class T>
    requires type_traits::is_serializable_nonptr<T>::value
class Channel {

    public:
        static Channel<T> make_chan() { return Channel<T>(); }

        template <class U>
            requires type_traits::is_serializable_nonptr<U>::value
        friend larrow<U> operator-(Channel<U>& chan);

        friend void operator<(Channel<T>& lhs, const larrow<T>& rhs) {
            std::lock_guard<std::mutex> lock(lhs.mtx);
            lhs.data.push(rhs.data);
        }
    private:
        std::queue<T> data;
        std::mutex mtx;
};

template <class T> void drain(Channel<T>& chan) {
    while (!chan.data.empty()) {
        std::println("{}", chan.data.front());
        chan.data.pop();
    }
}

template <class T>
    requires type_traits::is_serializable_nonptr<T>::value
T operator<(T& lhs, larrow<T> rhs) {
    lhs = rhs.data;
    return lhs;
}

class go {
    public:
        template <class F> go(F&& f) { std::thread(std::forward<F>(f)).detach(); }
};

int main() {
    auto chanInt = Channel<int>::make_chan();
    chanInt <- (42);

    auto chanString = Channel<std::string>::make_chan();
    chanString <- std::string("Hello, World!");

    go([&] { chanString <- std::string("not hi"); });

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(200ms);
    std::string i;
    i <- chanString;
    std::print("{}\n", i);
    i <- chanString;
    std::print("{}\n", i);
    return 0;
}
