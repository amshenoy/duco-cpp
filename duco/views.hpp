#pragma once
#include <chrono>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <string>

#include "data.hpp"

namespace duco::views
{
    using namespace std::chrono;

    auto constexpr b2i = [](auto const& c4) {
        uint32_t value = 0;
        int i = 0;
        for (auto it = c4.begin(); it != c4.end(); ++it, ++i) {
            value |= static_cast<uint32_t>(static_cast<uint8_t>(*it)) << (i * 8); // Read as little-endian
            // value |= static_cast<uint32_t>(static_cast<uint8_t>(*it)) << ((3-i) * 8); // Read as big-endian
        }
        return value;
    };

    auto constexpr r2t = [](int sym = 0) { return [sym](auto const& row){ return duco::data::TickRow(sym, row); }; };

    auto constexpr rows_view(int sym) {
        return std::views::chunk(4)
            | std::views::transform(b2i)
            | std::views::chunk(5)
            | std::views::transform(r2t(sym));
    }


}