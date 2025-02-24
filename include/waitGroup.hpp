#pragma once

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