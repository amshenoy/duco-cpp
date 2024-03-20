#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>

using namespace std::chrono;

class date_iterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = system_clock::time_point;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;

    constexpr date_iterator() : val_() {}
    constexpr date_iterator(value_type val) : val_(val) {}

    constexpr reference operator*() const { return val_; }
    constexpr date_iterator& operator++() {
        val_ += hours(24);
        return *this;
    }
    constexpr date_iterator operator++(int) {
        date_iterator temp = *this;
        ++(*this);
        return temp;
    }
    constexpr bool operator==(const date_iterator& other) const { return val_ == other.val_; }
    constexpr bool operator!=(const date_iterator& other) const { return !(*this == other); }

private:
    value_type val_;
};

class date_range : public std::ranges::view_interface<date_range>
{
public:
    using value_type = system_clock::time_point;

    constexpr date_range() = default;
    constexpr date_range(sys_days start, sys_days end) : start_(start), end_(end + days{1}) {}
    constexpr date_range(value_type start, value_type end) : start_(start), end_(end + days{1}) {}

    constexpr date_iterator begin() const { return date_iterator(start_); }
    constexpr date_iterator end() const { return date_iterator(end_); }

    constexpr operator std::ranges::subrange<date_iterator>() const { return {begin(), end()}; }

private:
    value_type start_;
    value_type end_;
};

// date_range_ymd is equivalent to doing the following:
// date_range period(start, end);
// auto dateRangeYmd = period
//     | std::views::transform([](auto const day){ return year_month_day{floor<days>(day)}; });
// auto dateRangeYmd = date_range(start, end) | std::views::transform([](auto const d){ return year_month_day{floor<days>(d)}; });
struct date_range_ymd {
    date_range period;

    std::function<year_month_day(typename date_range::value_type)> transform =
    [](auto const day){ return year_month_day{floor<days>(day)}; };
    
    std::ranges::transform_view<decltype(period), decltype(transform)> view;

    date_range_ymd(sys_days start, sys_days end)
        : period(start, end),
          view(std::views::transform(period, transform)) {}

    date_range_ymd(date_range range)
        : period(std::move(range)),
          view(std::views::transform(period, transform)) {}
    
    auto begin() const { return std::begin(view); }
    auto end() const { return std::end(view); }
};

// TODO: Fix this so it works with pipe operator std::views::transform
// date_range_ymd dateRangeYmd(start, end); 


// TODO: We do not need this since date_range is view-compatible
// struct date_range_view
// {
//     date_range range;
//     auto begin() const { return range.begin(); }
//     auto end() const { return range.end(); }
//     template<typename View>
//     auto operator|(View&& view) const { return range | std::forward<View>(view); }
//     // auto operator|(View&& view) const { return std::views::all(range) | std::forward<View>(view); }
// };
// struct make_date_range_view : std::ranges::range_adaptor_closure<make_date_range_view> {
//     auto operator()(date_range range) const { return date_range_view{range}; }
// };
// constexpr make_date_range_view date_range_to_view;



// TODO: Do not inline string_to_sys_days
inline sys_days string_to_sys_days(std::string const& date_string) {
    std::istringstream in{date_string};
    int year_i;
    unsigned int month_i, day_i;
    char delimiter;

    in >> year_i >> delimiter >> month_i >> delimiter >> day_i;

    year year{year_i};
    month month{month_i};
    day day{day_i};

    return sys_days{year/month/day};
}




// Usage:
// using namespace std::chrono;
// sys_days start = 2023y / November / 22d;
// sys_days end = 2023y / December / 31d;
// date_range period(start, end); // inclusive of start and end dates
// // auto date_range = date_range_to_view(period) | std::views::transform([](auto const day){
// auto date_range = period | std::views::transform([](auto const day){
//     return year_month_day{floor<days>(day)};
// });
// date_range_ymd dateRangeYmd(start, end);