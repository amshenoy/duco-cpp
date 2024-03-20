#pragma once

#include "sorted_merge_generator.hpp"

// IDEA 1: Use the existing SortedMergeGenerator as a base class
// Interesting but not the best
/*
template <typename View>
struct SortedMergeOwningGenerator : SortedMergeGenerator<View> {
    using Base = SortedMergeGenerator<View>;
    using Iterator = typename Base::Iterator;
    using Sentinel = typename Base::Sentinel;
    using ValueType = typename Base::ValueType;
    std::vector<View> views;

    SortedMergeOwningGenerator() : Base(), views() {}

    template <typename... Views>
    requires (std::ranges::range<Views> && ...)
    SortedMergeOwningGenerator(Views&&... views) : Base(views...), views(std::forward<Views>(views)...) {}

    SortedMergeOwningGenerator(std::vector<View>&& views) : Base(views), views(std::move(views)) {}

    void emplaceView(View& view, size_t index) override {
        auto begin = std::make_unique<Iterator>(std::ranges::begin(view));
        auto end = std::make_unique<Sentinel>(std::ranges::end(view));
        if (*begin != *end) {
            this->minHeap.emplace(*begin, index);
            this->iterators.emplace_back(std::move(begin));
            this->ends.emplace_back(std::move(end));
        }
    }


    // template <typename... Views>
    // requires (std::ranges::range<Views> && ...)
    // SortedMergeOwningGenerator(Views&&... views) : views(std::forward<Views>(views)...) {
    //     size_t i = 0;
    //     (Base::emplaceView(this->views[i], i++), ...);
    //     Base::value = std::numeric_limits<ValueType>::max();
    // }

    // SortedMergeOwningGenerator(std::vector<View>&& views) : views(std::move(views)) {
    //     size_t i = 0;
    //     for (View& view : this->views) Base::emplaceView(view, i++);
    //     Base::value = std::numeric_limits<ValueType>::max();
    // }

    // void emplaceView(View& view, size_t index) override {
    //     auto begin = std::make_unique<Iterator>(std::ranges::begin(view));
    //     auto end = std::make_unique<Sentinel>(std::ranges::end(view));
    //     if (*begin != *end) {
    //         Base::minHeap.emplace(*begin, index);
    //         Base::iterators.emplace_back(std::move(begin));
    //         Base::ends.emplace_back(std::move(end));
    //     }
    // }

};
*/


// IDEA 2: Replace the old one with a new one that moves views in constructor


template <typename View>
struct SortedMergeOwningGenerator {
    using Iterator = std::ranges::iterator_t<View>;
    using Sentinel = std::ranges::sentinel_t<View>;
    using ValueType = std::ranges::range_value_t<View>;
    std::vector<std::unique_ptr<Iterator>> iterators;
    std::vector<std::unique_ptr<Sentinel>> ends;
    ValueType value;
    std::priority_queue<std::pair<ValueType, size_t>, std::vector<std::pair<ValueType, size_t>>, std::greater<>> minHeap;

    ValueType getValue() const { return value; }

    SortedMergeOwningGenerator() : value(std::numeric_limits<ValueType>::max()) {}

    template <typename... Views>
    requires (std::ranges::range<Views> && ...)
    SortedMergeOwningGenerator(Views&... views) {
        size_t i = 0;
        (emplaceView(std::move(views), i++), ...);
        value = std::numeric_limits<ValueType>::max();
    }

    SortedMergeOwningGenerator(std::vector<View>& views) {
        size_t i = 0;
        for (View& view : views) emplaceView(std::move(view), i++);
        value = std::numeric_limits<ValueType>::max();
    }



    template <typename... Views>
    requires (std::ranges::range<Views> && ...)
    SortedMergeOwningGenerator(Views&&... views) {
        size_t i = 0;
        (emplaceView(std::forward<Views>(views), i++), ...);
        value = std::numeric_limits<ValueType>::max();
    }

    SortedMergeOwningGenerator(std::vector<View>&& views) {
        size_t i = 0;
        for (View& view : views) emplaceView(view, i++);
        value = std::numeric_limits<ValueType>::max();
    }


    

    virtual void emplaceView(View& view, size_t index) {
        auto begin = std::make_unique<Iterator>(std::ranges::begin(view));
        auto end = std::make_unique<Sentinel>(std::ranges::end(view));
        iterators.push_back(std::move(begin));
        ends.push_back(std::move(end));
        if (*iterators.back() != *ends.back()) minHeap.emplace(**iterators.back(), index);
    }

    virtual void emplaceView(View&& view, size_t index) {
        auto begin = std::make_unique<Iterator>(view.begin());
        auto end = std::make_unique<Sentinel>(view.end());
        iterators.push_back(std::move(begin));
        ends.push_back(std::move(end));
        if (*iterators.back() != *ends.back()) minHeap.emplace(**iterators.back(), index);
    }

    bool moveNext() {
        if (minHeap.empty()) return false;
        auto [minValue, idx] = minHeap.top();
        minHeap.pop();
        value = minValue;
        if (++*iterators[idx] != *ends[idx]) minHeap.emplace(**iterators[idx], idx);
        return true;
    }

};


template <typename View>
SortedMergeOwningGenerator(View&...) -> SortedMergeOwningGenerator<View>;

template <typename View>
SortedMergeOwningGenerator(View&&...) -> SortedMergeOwningGenerator<View>;

// Deduction guide for vector of views (lvalue references)
template <typename View>
SortedMergeOwningGenerator(std::vector<View>&) -> SortedMergeOwningGenerator<View>;

// Deduction guide for vector of views (rvalue references)
template <typename View>
SortedMergeOwningGenerator(std::vector<View>&&) -> SortedMergeOwningGenerator<View>;


// // Deduction guide for individual views (lvalue references)
// template <typename... Views>
// SortedMergeOwningGenerator(Views&...) -> SortedMergeOwningGenerator<std::decay_t<Views>...>;

// // Deduction guide for individual views (rvalue references)
// template <typename... Views>
// SortedMergeOwningGenerator(Views&&...) -> SortedMergeOwningGenerator<std::decay_t<Views>...>;



    
template <typename View>
class SortedMergeOwningGenView : public std::ranges::view_interface<SortedMergeOwningGenView<View>> {
public:
    using Generator = SortedMergeOwningGenerator<View>;
    SortedMergeOwningGenView() = default;
    explicit SortedMergeOwningGenView(Generator&& generator) : gen_(std::make_unique<Generator>(std::move(generator))) {}

    SortedMergeIterator<View> begin() const { return SortedMergeIterator<View>(*gen_); }
    SortedMergeIterator<View> end() const { return SortedMergeIterator<View>(); }
private:
    std::unique_ptr<Generator> gen_;
};
