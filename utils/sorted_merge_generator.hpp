#include <algorithm>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <queue>
#include <ranges>
#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <stdint.h>
#include <tuple>
#include <vector>


template <typename View>
struct SortedMergeGenerator {
    using Iterator = std::ranges::iterator_t<View>;
    using Sentinel = std::ranges::sentinel_t<View>;
    using ValueType = std::ranges::range_value_t<View>;
    std::vector<std::unique_ptr<Iterator>> iterators;
    std::vector<std::unique_ptr<Sentinel>> ends;
    ValueType value;
    std::priority_queue<std::pair<ValueType, size_t>, std::vector<std::pair<ValueType, size_t>>, std::greater<>> minHeap;

    constexpr ValueType getValue() const { return value; }

    constexpr SortedMergeGenerator() : value(std::numeric_limits<ValueType>::max()) {}

    template <typename... Views>
    requires (std::ranges::range<Views>&& ...)
    constexpr SortedMergeGenerator(Views&... views) {
        size_t i = 0;
        (emplaceView(views, i++), ...);
        value = std::numeric_limits<ValueType>::max();
    }

    constexpr SortedMergeGenerator(std::vector<View>& views) {
        size_t i = 0;
        for (View& view : views) emplaceView(view, i++);
        value = std::numeric_limits<ValueType>::max();
    }

    constexpr SortedMergeGenerator(std::vector<View>&& views) {
        size_t i = 0;
        for (View& view : views) emplaceView(std::move(view), i++);
        value = std::numeric_limits<ValueType>::max();
    }
    
    virtual constexpr void emplaceView(View& view, size_t index) {
        auto begin = std::make_unique<Iterator>(std::ranges::begin(view));
        auto end = std::make_unique<Sentinel>(std::ranges::end(view));
        iterators.push_back(std::move(begin));
        ends.push_back(std::move(end));
        if (*iterators.back() != *ends.back()) minHeap.emplace(**iterators.back(), index);
    }

    virtual constexpr void emplaceView(View&& view, size_t index) {
        auto begin = std::make_unique<Iterator>(std::ranges::begin(view));
        auto end = std::make_unique<Sentinel>(std::ranges::end(view));
        iterators.push_back(std::move(begin));
        ends.push_back(std::move(end));
        if (*iterators.back() != *ends.back()) minHeap.emplace(**iterators.back(), index);
    }
    
    // // New constructor taking rvalue references
    // TODO: UNCOMMENT AFTER FIXING RVALUE EMPLACE_VIEW
    // template <typename... Views>
    // requires (std::ranges::range<Views> && ...)
    // SortedMergeGenerator(Views&&... views) {
    //     size_t i = 0;
    //     (emplaceView(std::forward<Views>(views), i++), ...);
    //     value = std::numeric_limits<ValueType>::max();
    // }

    // TODO: FIX THIS SO THAT WE CAN USE THIS INSTEAD OF THE VECTOR CONSTRUCTOR
    // template <typename RangeOfRanges>
    // requires std::ranges::range<RangeOfRanges> && std::ranges::range<std::ranges::range_value_t<RangeOfRanges>>
    // SortedMergeGenerator(RangeOfRanges&& range_of_ranges) {
    //     size_t i = 0;
    //     for(auto& view : range_of_ranges) emplaceView(view, i++);
    //     value = std::numeric_limits<ValueType>::max();
    // }

    constexpr bool moveNext() {
        if (minHeap.empty()) return false;
        auto [minValue, idx] = minHeap.top();
        minHeap.pop();
        value = minValue;
        if (++*iterators[idx] != *ends[idx]) minHeap.emplace(**iterators[idx], idx);
        return true;
    }
};

template <typename View>
SortedMergeGenerator(View...) -> SortedMergeGenerator<View>;

template <typename View>
SortedMergeGenerator(std::vector<View>) -> SortedMergeGenerator<View>;

template <typename View>
SortedMergeGenerator(std::vector<View>&&) -> SortedMergeGenerator<View>;

template <typename... Views>
SortedMergeGenerator(Views&&...) -> SortedMergeGenerator<std::ranges::range_value_t<Views>...>;


// template <typename ValueType, typename... Views>
// auto make_generator(Views&&... views) {
//     return SortedMergeGenerator<ValueType, Views...>(std::forward<Views>(views)...);
// }
// // Usage:
// auto generator = make_generator<int>(v1, v2);

// TODO: Allow SortedMergeGenerator to take any type as long as the value_type
// of the range is the same for all the views
// Possibly may need to / want to / deduce value_type
// Type deduction guide for this version:
// template <typename... Views>
// SortedMergeGenerator(Views...) -> SortedMergeGenerator<Views...>;



template <typename Iterator>
class SortedMergeIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = typename SortedMergeGenerator<Iterator>::ValueType;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;

    constexpr SortedMergeIterator() : gen_(nullptr){};

    constexpr explicit SortedMergeIterator(SortedMergeGenerator<Iterator>& generator)
        : gen_(&generator) {
        moveNext();
    }

    // TODO: ADD RVALUE CTOR
    
    // constexpr explicit SortedMergeIterator(SortedMergeGenerator<Iterator>&& generator)
    //     : gen_(&std::move(generator)) {
    //     moveNext();
    // }

    // TODO: USE UNIQUE POINTER
    // constexpr explicit SortedMergeIterator(SortedMergeGenerator<Iterator>&& gen) 
    //     : gen_(std::make_unique<SortedMergeGenerator<Iterator>>(std::move(gen))) {
    //         moveNext();    
    //     }

    constexpr reference operator*() const { return current_; }
    constexpr pointer operator->() const { return &current_; }

    constexpr SortedMergeIterator& operator++() {
        moveNext();
        return *this;
    }

    constexpr SortedMergeIterator operator++(int) {
        SortedMergeIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr friend bool operator==(const SortedMergeIterator& lhs, const SortedMergeIterator& rhs) { return lhs.gen_ == rhs.gen_; }
    constexpr friend bool operator!=(const SortedMergeIterator& lhs, const SortedMergeIterator& rhs) { return !(lhs == rhs); }
private:
    SortedMergeGenerator<Iterator>* gen_ = nullptr;
    value_type current_;

    constexpr void moveNext() {
        if (gen_ && gen_->moveNext()) { current_ = gen_->getValue(); }
        else { gen_ = nullptr; }
    }
};

template <typename Iterator>
class SortedMergeView : public std::ranges::view_interface<SortedMergeView<Iterator>> {
public:
    constexpr SortedMergeView() = default;
    constexpr explicit SortedMergeView(SortedMergeGenerator<Iterator>& generator) : gen_(&generator) {}
    constexpr SortedMergeIterator<Iterator> begin() const { return SortedMergeIterator<Iterator>(*gen_); }
    constexpr SortedMergeIterator<Iterator> end() const { return SortedMergeIterator<Iterator>(); }
private:
    SortedMergeGenerator<Iterator>* gen_ = nullptr;
};


// TAKES OWNERSHIP OF GENERATOR VIA UNIQUE POINTER
template <typename Iterator>
class SortedMergeOwningView : public std::ranges::view_interface<SortedMergeOwningView<Iterator>> {
public:
    constexpr SortedMergeOwningView() = default;
    constexpr explicit SortedMergeOwningView(SortedMergeGenerator<Iterator>&& generator) : gen_(std::make_unique<SortedMergeGenerator<Iterator>>(std::move(generator))) {}

    // TODO: FIX THIS    
    // template <typename... Views>
    // explicit SortedMergeOwningView(Views&&... views)
    //     : gen_(std::make_unique<SortedMergeGenerator<Iterator>>(std::forward<Views>(views)...)) {}

    // TODO: FIX THIS, THIS MIGHT BE USEFUL
    // explicit SortedMergeOwningView(std::vector<std::ranges::views::all_t<decltype(*std::declval<Iterator>())>>& views)
    //     : gen_(std::make_unique<SortedMergeGenerator<Iterator>>(views)) {}

    constexpr SortedMergeIterator<Iterator> begin() const { return SortedMergeIterator<Iterator>(*gen_); }
    constexpr SortedMergeIterator<Iterator> end() const { return SortedMergeIterator<Iterator>(); }
private:
    std::unique_ptr<SortedMergeGenerator<Iterator>> gen_;
};


// USAGE EXAMPLE
/*
int main() {
    std::vector<int> range1 = {1, 4, 7, 10};
    std::vector<int> range2 = {2, 5, 8, 11};
    std::vector<int> range3 = {3, 6, 9, 12};

    SortedMergeGenerator<std::vector<int>::iterator> generator(range1, range2, range3);
    SortedMergeView view(std::move(generator)); // SortedMergeView<std::vector<int>::iterator>
    auto testView = view | std::views::filter([](auto element){ return element % 2 == 0; });
    
    for (auto value : testView) {
        std::cout << value << " ";
    }
    return 0;
}
*/


// struct make_sorted_range_view : std::ranges::range_adaptor_closure<make_sorted_range_view>
// {
//     auto operator()(auto iters) const { return SortedMergeView{iters}; }
// };
// constexpr make_sorted_range_view sorted_range_to_view;

// // USAGE:
// auto view = SortedMergeView<std::vector<int>::iterator>{{std::begin(range1), std::end(range1)}, {std::begin(range2), std::end(range2)}, {std::begin(range3), std::end(range3)}};
// auto view = sorted_range_to_view({{std::begin(range1), std::end(range1)}, {std::begin(range2), std::end(range2)}, {std::begin(range3), std::end(range3)}});





template <typename View>
class CombinedSortedView;

template <typename View>
class CombinedSortedView : public std::ranges::view_interface<CombinedSortedView<View>> {
public:
    using Generator = SortedMergeGenerator<View>;
    
    constexpr CombinedSortedView() = default;
    
    constexpr explicit CombinedSortedView(std::vector<View>& views) 
        : gen_(views), begin_(gen_), end_() {}

    constexpr explicit CombinedSortedView(std::vector<View>&& views) 
        : gen_(std::move(views)), begin_(gen_), end_() {}

    constexpr auto begin() const { return begin_; }
    constexpr auto end() const { return end_; }
    
private:
    Generator gen_;
    SortedMergeIterator<View> begin_;
    SortedMergeIterator<View> end_;
};

template <typename View>
CombinedSortedView(std::vector<View>&) -> CombinedSortedView<View>;

template <typename View>
CombinedSortedView(std::vector<View>&&) -> CombinedSortedView<View>;

// template <typename View>
// auto operator|(std::vector<View> views, CombinedSortedView<View>) {
//     return CombinedSortedView(views);
// }

template <typename View>
auto operator|(std::vector<View>& views, CombinedSortedView<View>) -> CombinedSortedView<View> {
    return CombinedSortedView<View>(views);
}


template <typename View>
auto operator|(std::vector<View>&& views, CombinedSortedView<View>) -> CombinedSortedView<View> {
    return CombinedSortedView(std::move(views));
}



// Struct for combined_sorted
struct combined_sorted {};

// Operator | overload for pipe operator
template <typename View>
auto operator|(std::vector<View>& views, combined_sorted) -> CombinedSortedView<View> {
    return CombinedSortedView(views);
}


template <typename View>
auto operator|(std::vector<View>&& views, combined_sorted) -> CombinedSortedView<View> {
    return CombinedSortedView(std::move(views));
}







// ----- GENERIC EXAMPLE FOR TESTING

// std::vector<int> const v1 {1, 5, 8};
// std::vector<int> const v2 {2, 3};
// auto const testView = std::views::cartesian_product(v1, v2)
//     | std::views::transform([](auto const& tup){ return std::views::iota(std::get<0>(tup)) | std::views::take(std::get<1>(tup)); })
//     | std::views::chunk(2);

// auto sortedMergeViews = testView
//     | std::views::transform([](auto const& view){ 
//         auto vec = view | std::ranges::to<std::vector>();
//         return SortedMergeOwningView(SortedMergeGenerator(vec));
//     });

// for (auto const& sortView : sortedMergeViews) {
//     for (auto const& val : sortView)
//         std::cout << val << std::endl; 
// }

// ---------
// // SortedMergeOwningView CHECKS AND TESTS
// std::vector<int> ints {4, 8, 11, 12, 19, 7, 16, 20, 23, 24, 7, 8, 12, 14, 18, 3, 14, 21};

// CHECK 1 - WORKING
// auto views = ints | std::views::chunk(5) | std::ranges::to<std::vector>();
// // auto gen = SortedMergeGenerator(views);
// // while (gen.moveNext()) { std::cout << gen.getValue() << std::endl; }
// // auto sortedMergeViews = SortedMergeOwningView(std::move(gen));
// auto sortedMergeViews = SortedMergeOwningView(SortedMergeGenerator(views));
// for (auto value : sortedMergeViews) { std::cout << value << std::endl; }

// CHECK 2 - WORKING
// auto viewsOfViews = ints | std::views::chunk(5) | std::views::chunk(2);
// auto sortedMergeViews = viewsOfViews
//     | std::views::transform([](auto const& views){
//         auto vec = views | std::ranges::to<std::vector>();
//         return SortedMergeOwningView(SortedMergeGenerator(vec));
//     });
// // for (auto value : sortedMergeViews | std::views::join) { std::cout << value << std::endl; }
// for (auto sortedView : sortedMergeViews) { 
//     for (auto value : sortedView) std::cout << value << ", ";
//     std::cout << std::endl;
// }
// ---------    

// SortedMergeOwningView view(rows1, rows2); // DOES NOT WORK YET; TODO: Fix SortedMergeOwningView
// auto gen = SortedMergeGenerator(rows1, rows2);
// SortedMergeOwningView view(std::move(gen));
// SortedMergeOwningView view(SortedMergeGenerator(rows1, rows2));
// for (auto value : view) { std::cout << value << std::endl; }

