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

// #include "duco/duco.hpp"
#include "duco/views.hpp"
// #include "utils/concat_view.hpp"
// #include "utils/sorted_merge_generator.hpp"
#include "utils/sorted_merge_owning_generator.hpp"

#include "utils/date_range.hpp"

// using namespace duco;
using namespace std::chrono;

int main() {
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

    auto constexpr dateRange = date_range(start, end) | std::views::transform([](auto const d){ return year_month_day{floor<days>(d)}; });
    // date_range_ymd constexpr dateRange(start, end); // TODO: Fix this as well
    // auto constexpr dateRange = date_range_ymd(start, end); // TODO: Create this working alternative


    auto dayGroupedViews = std::views::cartesian_product(dateRange, symbols)
        | std::views::transform([](auto const& tup){
                return duco::views::asset_day_view(std::get<1>(tup), std::get<0>(tup)) 
                    | std::views::take(5); // Comment this out after development and testing
            })
        | std::views::chunk(symbols.size());

    // ---------
    // std::vector<int> testData {1,2,4,3,6,9,5,7,8};
    // auto testViews = testData | std::views::chunk(3); 
    // auto genViews = testViews | std::views::transform([](auto const& view){});
    // ---------
    // GENERATOR VIEWS CHECK

    // TODO: Not working, the view is not being owned by the generator
    // auto dayGeneratorViews = dayGroupedViews
    //     | std::views::transform([](auto const& dayGroupView){
    //         auto vec = dayGroupView | std::ranges::to<std::vector>();
    //         return SortedMergeGenerator(vec);
    //     });

    // TODO:  Not working either, generator needs to take ownership of the view
    // auto dayGeneratorViews = dayGroupedViews
    //     | std::views::transform([](auto const& dayGroupView){
    //         auto vec = dayGroupView | std::ranges::to<std::vector>();
    //         // return SortedMergeOwningGenerator(vec);
    //         return SortedMergeOwningGenerator(std::move(vec));
    //     });
    // for (auto gen : dayGeneratorViews) {
    //     while (gen.moveNext()) { std::cout << gen.getValue() << std::endl; }
    // }

    // WORKING BUT WITHOUT SORTED MERGE
    // std::cout << "Without Sorted Merge" << std::endl;
    // auto dayVecViews = dayGroupedViews
    //     | std::views::transform([](auto const& dayGroupView){
    //         auto vec = dayGroupView | std::ranges::to<std::vector>();
    //         return vec;
    //     });
    // for (auto dayView : dayVecViews) {
    //     for (auto& assetView : dayView) {
    //         for (auto value : assetView) std::cout << value << std::endl;
    //     }
    // }

    // ---------
    // METHOD 1: NOT YET WORKING
    // auto daySortedViews = dayGroupedViews
    //     | std::views::transform([](auto const& dayGroupView){
    //         auto vec = dayGroupView | std::ranges::to<std::vector>();
    //         return SortedMergeOwningView(SortedMergeGenerator(vec));
    //     });
    
    // METHOD 1, STEP 2: Explicit
    // for (auto daySortedView : daySortedViews) {
    //     for (auto value : daySortedView) std::cout << value << std::endl;
    // }
    
    // METHOD 1, STEP 2 ALTERNATIVE: Simplifcation
    // for (auto const& value : daySortedViews | std::views::join) std::cout << value << std::endl; 



    // METHOD 2: WORKING
    for (auto dayGroupView : dayGroupedViews) {
        auto vec = dayGroupView | std::ranges::to<std::vector>();
        // std::cout << vec.size() << std::endl; // Should be equivalent to num of symbols
        auto daySortedView = SortedMergeOwningView(SortedMergeGenerator(vec)); // WORKING
        // auto daySortedView = SortedMergeOwningGenView(SortedMergeOwningGenerator(vec)); // TESTING
        for (auto value : daySortedView) std::cout << value << std::endl;
    }

    // auto dayMergedViews = dateRange
    //     | std::views::transform([&symbols](auto const& d){
    //         using ViewType = decltype(duco::views::asset_day_view(std::declval<decltype(symbols.front())>(), d) | std::views::take(5));
    //         std::vector<ViewType> test;
    //         for (auto const& s : symbols)
    //         {
    //             auto view = duco::views::asset_day_view(s, d) | std::views::take(5);
    //             test.push_back(view);
    //         }
    //         return SortedMergeOwningView(SortedMergeGenerator(test));
    //     });
    // for (auto value : dayMergedViews) { std::cout << value << std::endl; }
    // for (auto dayView : dayMergedViews) {
    //     for (auto value : dayView) std::cout << value << std::endl; 
    // }
}
