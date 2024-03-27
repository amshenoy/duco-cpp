#include <algorithm>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
// #include <range/v3/all.hpp>
// #include <fmt/core.h>
// #include <tuple>
// #include <vector>

#include "duco/views.hpp"
// #include "utils/sorted_merge_generator.hpp"
#include "utils/sorted_merge_owning_generator.hpp"

#include "utils/date_range.hpp"

// using namespace duco;
using namespace std::chrono;

int main() {

    typedef std::chrono::high_resolution_clock Clock;

    // auto constexpr symbol1 = "EURUSD";
    // auto constexpr symbol2 = "GBPUSD";
    // auto constexpr start = 2023y / November / 01d;
    // // auto constexpr end = 2023y / November / 03d;
    // auto constexpr end = 2023y / November / 21d;
    // std::vector<std::string> const symbols{symbol1, symbol2};

    // auto constexpr symbol1 = "EURCHF";
    auto constexpr symbol1 = "EURUSD";
    auto constexpr symbol2 = "GBPUSD";
    auto constexpr start = 2023y / November / 02d;
    auto constexpr end = 2023y / November / 03d;
    std::vector<std::string> const symbols{symbol1, symbol2};

    auto t1 = Clock::now();

    auto constexpr dateRange = date_range(start, end) | std::views::transform([](auto const d){ return year_month_day{floor<days>(d)}; });

    auto t2 = Clock::now();

    auto dayGroupedViews = std::views::cartesian_product(dateRange, symbols)
        | std::views::transform([](auto const& tup){
                return duco::views::asset_day_view(std::get<1>(tup), std::get<0>(tup)); 
                    // | std::views::take(5); // Comment this out after development and testing
            })
        | std::views::chunk(symbols.size());

    auto t3 = Clock::now();
    

    // WORKING BUT WITHOUT SORTED MERGE
    // std::cout << "Without Sorted Merge" << std::endl;
    auto dayVecViews = dayGroupedViews
        | std::views::transform([](auto const& dayGroupView){
            return dayGroupView | std::ranges::to<std::vector>();
        });
    
    auto t4 = Clock::now();

    long long count1 = 0;

    for (auto dayView : dayVecViews) {
        for (auto& assetView : dayView) {
            // for (auto value : assetView) std::cout << value << std::endl;
            for (auto const& value : assetView) count1 += 1;
        }
    }

    auto t5 = Clock::now();


    long long count2 = 0;

    // METHOD 2: WORKING
    for (auto dayGroupView : dayGroupedViews) {
        auto vec = dayGroupView | std::ranges::to<std::vector>();
        // // std::cout << vec.size() << std::endl; // Should be equivalent to num of symbols
        // // auto daySortedView = SortedMergeOwningView(SortedMergeGenerator(vec)); // WORKING
        // // auto daySortedView = CombinedSortedView(vec); // WORKING
        // // auto daySortedView = vec | CombinedSortedView<std::ranges::range_value_t<decltype(vec)>>{}; // WORKING
        auto daySortedView = vec | combined_sorted{}; // WORKING

        // auto daySortedView = dayGroupView | std::ranges::to<std::vector>() | combined_sorted{}; // NOT WORKING
        
        // auto daySortedView = SortedMergeOwningGenView(SortedMergeOwningGenerator(vec)); // TESTING
        // for (auto value : daySortedView) std::cout << value << std::endl;
        for (auto value : daySortedView) count2 += 1;
    }


    // Method 2: NOT WORKING Alternative
    // auto daySortedViews = dayGroupedViews
    //     | std::views::transform([](auto dayGroupView){
    //         auto vec = dayGroupView | std::ranges::to<std::vector>();
    //         return SortedMergeOwningView(SortedMergeGenerator(vec));
    //     });
    // for (auto daySortedView : daySortedViews) {
    //     for (auto value : daySortedView) std::cout << value << std::endl; //count2 += 1;
    // }

    auto t6 = Clock::now();


    std::cout << "COUNT1: " << count1 << std::endl;
    std::cout << "COUNT2: " << count2 << std::endl;
    std::cout << "Date Range: " << duration_cast<nanoseconds>(t2-t1) << '\n';
    std::cout << "Day Grouped Views: " << duration_cast<nanoseconds>(t3-t2) << '\n';
    std::cout << "Day Vec Views: " << duration_cast<nanoseconds>(t4-t3) << '\n';
    std::cout << "Iteration Loop: " << duration_cast<nanoseconds>(t5-t4) << '\n';
    std::cout << "Sorted Iteration Loop: " << duration_cast<nanoseconds>(t6-t5) << '\n';
    std::cout << std::endl;
    std::cout << "Total Duration: " << duration_cast<nanoseconds>(t6-t1) << '\n';

}
