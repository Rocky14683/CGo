#pragma once
#include <queue>
#include <utility>
#include <type_traits>

#include <string>
#include <type_traits>
#include <thread>
#include <mutex>

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

// class ChanError : public std::exception {
//     public:
//         ChanError(const char* msg) : std::exception() {}
//
//         const char* what() const noexcept override { return msg.c_str(); }
//     private:
//         std::string msg;
// };

template <class T>
    requires type_traits::is_serializable_nonptr<T>::value
class Channel;

template <class T> struct larrow {
        template <class U = T>
            requires(!std::is_signed_v<T> || !std::is_arithmetic_v<T>)
        larrow(const T& x) : data(std::move(x)) {}

        template <class U = T>
            requires std::is_signed<U>::value && std::is_arithmetic<U>::value
        larrow(const U& x) : data(-x) {}

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
        template <class U = T> larrow_out(const T& x) : data(std::move(x)) {}

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
        static Channel<T> make_chan(std::optional<size_t> size = std::nullopt) { return Channel<T>(size); }

        Channel<T>(std::optional<size_t> q_size = std::nullopt) : q_size(q_size) {};

        Channel<T>(const Channel<T>&) = delete;

        Channel<T>& operator=(const Channel<T>&) = delete;

        Channel<T>(Channel<T>&&) = delete;

        Channel<T>& operator=(Channel<T>&&) = delete;

        template <class U>
            requires type_traits::is_serializable_nonptr<U>::value
        friend larrow_out<U> operator-(Channel<U>& chan);

        friend void operator<(Channel<T>& lhs, const larrow<T>& rhs) {
            if (lhs.is_closed) { throw std::runtime_error("Channel is closed"); }
            std::unique_lock<std::mutex> lock(lhs.mtx);
            // sends to a buffered channel block only when the buffer is full.
            if (lhs.q_size.has_value() && lhs.q_datas.size() == lhs.q_size.value()) {
                lhs.cv.wait(lock, [&lhs] { return lhs.q_datas.size() < lhs.q_size.value(); });
            }
            lhs.q_datas.push_back(rhs.get_data());
            lhs.cv.notify_one();
        }


        /*
         * Iterator so that it can be used in for each loop
         */

        using iterator = typename std::deque<T>::iterator;


        iterator begin() {
            std::lock_guard<std::mutex> lock(this->mtx);
            return this->q_datas.begin();
        }
        iterator end() {
            std::lock_guard<std::mutex> lock(this->mtx);
            return this->q_datas.end();
        }

//        using const_iterator = typename std::deque<T>::const_iterator;
//        const_iterator begin() const { return this->q_datas.begin(); }
//        const_iterator end() const { return this->q_datas.end(); }

        inline constexpr void close() {
            std::lock_guard<std::mutex> lock(mtx);
            is_closed = true;
        }

    private:
        const std::optional<size_t> q_size;
        std::condition_variable cv;
        std::deque<T> q_datas;
        std::mutex mtx;
        std::atomic_bool is_closed {false};
};

template <class U>
    requires type_traits::is_serializable_nonptr<U>::value
larrow_out<U> operator-(Channel<U>& chan) {
    if (chan.is_closed) { throw std::runtime_error("Channel is closed"); }
    std::unique_lock<std::mutex> lock(chan.mtx);
    // receives block when the buffer is empty.
    chan.cv.wait(lock, [&chan] { return !chan.q_datas.empty(); });
    auto data = chan.q_datas.front();
    chan.q_datas.pop_front();
    return larrow_out<U>(data);
}

template <class T> void drain(Channel<T>& chan) {
    std::lock_guard<std::mutex> lock(chan.mtx);
    while (!chan.q_datas.empty()) {
        std::println("{}", chan.q_datas.front());
        chan.q_datas.pop_front();
    }
}

template <class T>
    requires type_traits::is_serializable_nonptr<T>::value
T operator<(T& lhs, larrow_out<T> rhs) {
    lhs = rhs.get_data();
    return lhs;
}