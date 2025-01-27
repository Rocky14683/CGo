#include <iostream>
#include <queue>
#include <utility>
#include <type_traits>

#include <string>
#include <type_traits>



namespace type_traits {
//SFINAE stuffs
template <class T> struct is_serializable : std::false_type {};

template <class T>
requires requires(T x) {
    { std::declval<std::ostream&>() << x };
} struct is_serializable<T> : std::true_type {};

template <class T> struct is_serializable_nonptr
    : std::bool_constant<!std::is_pointer_v<T> && is_serializable<T>::value> {};
};



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
class Channel {
    public:
        std::queue<T> data;

        static Channel<T> make_chan() { return Channel<T>(); }

        friend void operator<(Channel<T>& lhs, const larrow<T>& rhs) { lhs.data.push(rhs.data); }
};

template <class T> void drain(Channel<T>& chan) {
    while (!chan.data.empty()) {
        std::println("{}", chan.data.front());
        chan.data.pop();
    }
}

int main() {
    auto chanInt = Channel<int>::make_chan();
    chanInt <- 42;
    drain(chanInt);

    auto chanString = Channel<std::string>::make_chan();
    chanString <- std::string("Hello, World!");
    drain(chanString);

    return 0;
}
