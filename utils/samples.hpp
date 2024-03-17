#pragma once
#include <algorithm>
#include <chrono>
#include <concepts>
#include <coroutine>
#include <iostream>
#include <iterator>
#include <mutex>
#include <optional>
#include <ranges>
#include <span>
#include <sstream>
#include <thread>
#include <vector>


template <typename Range>
void print(Range const& row) {
    std::cout << "(";
    bool first = true;
    for (auto const& element : row) {
        if (!first) { std::cout << ", "; }
        std::cout << element;
        first = false;
    }
    std::cout << ")" << std::endl;
}

// // Helper function to construct a range from a pointer and a size
// template<typename Range, typename T>
// Range make_range(T* ptr, std::size_t size);

// // Specialization for std::span
// template<typename T>
// std::span<T> make_range(T* ptr, std::size_t size) {
//     return std::span<T>(ptr, size);
// }

// // Specialization for std::vector
// template<typename T>
// std::vector<T> make_range(T* ptr, std::size_t size) {
//     return std::vector<T>(ptr, ptr + size);
// }




// Helper struct to construct a range from a pointer and a size
template<typename Range, typename T>
struct RangeMaker {
    static Range make(T* ptr, std::size_t size);
};

// Specialization for std::span
template<typename T>
struct RangeMaker<std::span<T>, T> {
    static std::span<T> make(T* ptr, std::size_t size) {
        return std::span<T>(ptr, size);
    }
};

// Specialization for std::vector
template<typename T>
struct RangeMaker<std::vector<T>, T> {
    static std::vector<T> make(T* ptr, std::size_t size) {
        return std::vector<T>(ptr, ptr + size);
    }
};






// class generator {
// public:
//     struct promise_type;
//     using handle_type = std::coroutine_handle<promise_type>;

//     generator(handle_type h) : coro(h) {}
//     generator(const generator&) = delete;
//     generator(generator&& rhs) : coro(rhs.coro) { rhs.coro = nullptr; }
//     ~generator() { if (coro) coro.destroy(); }

//     uint8_t operator()() {
//         coro.resume();
//         return coro.promise().current_value;
//     }

//     struct promise_type {
//         uint8_t current_value;
//         auto get_return_object() { return generator{handle_type::from_promise(*this)}; }
//         auto initial_suspend() { return std::suspend_always{}; }
//         auto final_suspend() noexcept { return std::suspend_always{}; }
//         void return_void() {}
//         void unhandled_exception() { std::terminate(); }
//         auto yield_value(uint8_t value) {
//             current_value = value;
//             return std::suspend_always{};
//         }
//     };

// private:
//     handle_type coro;
// };




// https://en.cppreference.com/w/cpp/coroutine/coroutine_handle
template<std::movable T>
class Generator
{
public:
    struct promise_type
    {
        Generator<T> get_return_object()
        {
            return Generator{Handle::from_promise(*this)};
        }
        static std::suspend_always initial_suspend() noexcept
        {
            return {};
        }
        static std::suspend_always final_suspend() noexcept
        {
            return {};
        }
        std::suspend_always yield_value(T value) noexcept
        {
            current_value = std::move(value);
            return {};
        }
        // Disallow co_await in generator coroutines.
        void await_transform() = delete;
        [[noreturn]]
        static void unhandled_exception() { throw; }
 
        std::optional<T> current_value;
    };
 
    using Handle = std::coroutine_handle<promise_type>;
 
    explicit Generator(const Handle coroutine) :
        m_coroutine{coroutine}
    {}
 
    Generator() = default;
    ~Generator()
    {
        if (m_coroutine)
            m_coroutine.destroy();
    }
 
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
 
    Generator(Generator&& other) noexcept :
        m_coroutine{other.m_coroutine}
    {
        other.m_coroutine = {};
    }
    Generator& operator=(Generator&& other) noexcept
    {
        if (this != &other)
        {
            if (m_coroutine)
                m_coroutine.destroy();
            m_coroutine = other.m_coroutine;
            other.m_coroutine = {};
        }
        return *this;
    }
 
    // Range-based for loop support.
    class Iter
    {
    public:
        void operator++()
        {
            m_coroutine.resume();
        }
        const T& operator*() const
        {
            return *m_coroutine.promise().current_value;
        }
        bool operator==(std::default_sentinel_t) const
        {
            return !m_coroutine || m_coroutine.done();
        }
 
        explicit Iter(const Handle coroutine) :
            m_coroutine{coroutine}
        {}
 
    private:
        Handle m_coroutine;
    };
 
    Iter begin()
    {
        if (m_coroutine)
            m_coroutine.resume();
        return Iter{m_coroutine};
    }
 
    std::default_sentinel_t end() { return {}; }

    // Implicit conversion operator to std::vector<T>
    operator std::vector<T>() const {
        return {begin(), end()};
    }

private:
    Handle m_coroutine;
};


// // Range adaptor for Generator
// template <typename T>
// struct GeneratorRange {
//     Generator<T>& generator;

//     auto begin() const { return generator.begin(); }
//     auto end() const { return generator.end(); }

//     template <typename Func>
//     auto operator|(Func func) const {
//         return std::views::transform(generator, func);
//     }
// };

// template <typename T>
// GeneratorRange<T> view_from_generator(Generator<T>& generator) {
//     return {generator};
// }

// Example Usage:
// template<std::integral T>
// Generator<T> range(T first, const T last)
// {
//     while (first < last)
//         co_yield first++;
// }








/*
template <typename T>
auto merge_view = []<std::ranges::viewable_range... Views>(Views... views) {
    std::vector<T> v;
    ((v.insert(v.end(), views.begin(), views.end())), ...);
    std::ranges::sort(v);
    return std::views::all(v);
};
*/

