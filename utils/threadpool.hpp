#pragma once

#include <mutex>
#include <thread>
#include <vector>

class Threadpool
{
public:
    Threadpool(int numThreads) : mNumThreads(numThreads) {}

    template <typename Range, typename Function>
    void runTasks(Range range, Function func) {
        auto begin = range.begin();
        auto end = range.end();
        std::vector<std::jthread> threads;

        for (int i = 0; i < mNumThreads; ++i) {
            threads.emplace_back([this, &begin, &end, &func] {
                while (true) {
                    std::unique_lock<std::mutex> lock(this->mMutex);
                    if (begin == end) break;  // No more tasks

                    auto arguments = *begin;
                    // std::cout << "Thread " << std::this_thread::get_id() << " : " << arguments << std::endl;
                    ++begin;
                    lock.unlock();

                    std::apply(func, arguments);
                }
            });
        }
    }
private:
    int mNumThreads;
    std::mutex mMutex;
};
