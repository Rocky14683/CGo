#include <iostream>
#include <queue>

template <class T> struct larrow {
        larrow(T* a_) : a(a_) {}
        T* a;
};

template <class T> larrow<T> operator-(T& a) { return larrow<T>(&a); }


template <class T> class Channel {
    public:
        std::queue<T> data;

        Channel() : data() {}

        static Channel<std::string> make_chan() { return Channel<T> {}; }

        template <class U> friend void operator<(Channel<U>& lhs, const larrow<U>& it);
};

template <class U> void operator<(Channel<U>& lhs, const larrow<U>& it) {
    std::print("pushing {}\n", *it.a);
    return lhs.data.push(*it.a);
}

int main() {
    auto chan = Channel<std::string>::make_chan();
    std::string s = "hello";
    chan <- s;
}